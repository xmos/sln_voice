
.. _sln_voice_ffd_configuration:

Configuring the Firmware
========================

The default application performs as described in the :ref:`sln_voice_ffd_overview`. There are numerous compile time options that can be added to change the example design without requiring code changes.  To change the options explained in the table below, add the desired configuration variables to the APP_COMPILE_DEFINITIONS cmake variable in the ``.cmake`` file located in the ``examples/ffd/`` folder.

If options are changed, the application firmware must be rebuilt.

.. list-table:: FFD Compile Options
   :widths: 90 85 20
   :header-rows: 1
   :align: left

   * - Compile Option
     - Description
     - Default Value
   * - appconfINTENT_ENABLED
     - Enables/disables the intent engine, primarily for debug.
     - 1
   * - appconfINTENT_RESET_DELAY_MS
     - Sets the period after the wake up phrase has been heard for a valid command phrase
     - 5000
   * - appconfINTENT_RAW_OUTPUT
     - Set to 1 to output all keywords found, skipping the internal wake up and command state machine
     - 0
   * - appconfAUDIO_PLAYBACK_ENABLED
     - Enables/disables the audio playback command response
     - 1
   * - appconfINTENT_UART_OUTPUT_ENABLED
     - Enables/disables the UART intent message
     - 1
   * - appconfINTENT_UART_DEBUG_INFO_ENABLED
     - Enables/disables the UART intent debug information
     - 0
   * - appconfI2C_MASTER_DAC_ENABLED
     - Enables/disables configuring the DAC over |I2C| master
     - 1
   * - appconfINTENT_I2C_MASTER_OUTPUT_ENABLED
     - Enables/disables sending the intent message over |I2C| master
     - 1
   * - appconfINTENT_I2C_MASTER_DEVICE_ADDR
     - Sets the address of the |I2C| device receiving the intent via the |I2C| master interface
     - 0x01
   * - appconfINTENT_I2C_SLAVE_POLLED_ENABLED
     - Enables/disables allowing polling the intent message via |I2C| slave
     - 0
   * - appconfI2C_SLAVE_DEVICE_ADDR
     - Sets the address of the |I2C| device receiving the intent via the |I2C| slave interface
     - 0x42
   * - appconfINTENT_I2C_REG_ADDRESS
     - Sets the address of the |I2C| register to store the intent message, this value can be read via the |I2C| slave interface
     - 0x01
   * - appconfUART_BAUD_RATE
     - Sets the baud rate for the UART tx intent interface
     - 9600
   * - appconfUSE_I2S_INPUT
     - Replace |I2S| audio source instead of the microphone array audio source.
     - 0
   * - appconfI2S_MODE
     - Select |I2S| mode, supported values are appconfI2S_MODE_MASTER and appconfI2S_MODE_SLAVE
     - master
   * - appconfI2S_AUDIO_SAMPLE_RATE
     - Select the sample rate of the |I2S| interface, supported values are 16000 and 48000
     - 16000
   * - appconfRECOVER_MCLK_I2S_APP_PLL
     - Enables/disables the recovery of the MCLK from the Software PLL application; this removes the need to use an external MCLK.
     - 0
   * - appconfINTENT_TRANSPORT_DELAY_MS
     - Sets the delay between host wake up requested and |I2C| and UART keyword code transmission
     - 50
   * - appconfINTENT_QUEUE_LEN
     - Sets the maximum number of detected intents to hold while waiting for the host to wake up
     - 10
   * - appconfINTENT_WAKEUP_EDGE_TYPE
     - Sets the host wake up pin GPIO edge type.  0 for rising edge, 1 for falling edge
     - 0
   * - appconfAUDIO_PIPELINE_SKIP_IC_AND_VNR
     - Enables/disables the IC and VNR
     - 0
   * - appconfAUDIO_PIPELINE_SKIP_NS
     - Enables/disables the NS
     - 0
   * - appconfAUDIO_PIPELINE_SKIP_AGC
     - Enables/disables the AGC
     - 0

.. note::

  The ``example_ffd_i2s_input_cyberon`` has different default values from the ones in the table above.
  The list of updated values can be found in the ``APP_COMPILE_DEFINITIONS`` list in ``examples\ffd\ffd_i2s_input_cyberon.cmake``.

Configuring the |I2C| interfaces
--------------------------------

The |I2C| interfaces are used to configure the DAC and to communicate with the host. The |I2C| interface can be configured as a master or a slave.
The DAC must be configured at bootup via the |I2C| master interface.
The |I2C| master is used when the FFD example asynchronously sends intent messages to the host.  The |I2C| slave is used when the host wants to read intent messages from the FFD example through polling.

.. note::
  The |I2C| interface cannot operate as both master and slave simultaneously. The FFD example design uses the |I2C| master interface to configure the DAC at device initialisation.
  However, if the host reads intent messages from the FFD example using the |I2C| slave interface, the |I2C| master interface will be disabled after the DAC configuration is complete.

To send the intent ID via the |I2C| master interface when a command is detected, set the following variables:

  - ``appconfINTENT_I2C_MASTER_OUTPUT_ENABLED`` to 1.
  - ``appconfINTENT_I2C_MASTER_DEVICE_ADDR`` to the desired address used by the |I2C| slave device.
  - ``appconfINTENT_I2C_SLAVE_POLLED_ENABLED`` to 0, this will disable the |I2C| slave interface.

To configure the FFD example so that the host can poll for the intent via the |I2C| slave interface, set the following variables:

  - ``appconfINTENT_I2C_SLAVE_POLLED_ENABLED`` to 1.
  - ``appconfI2C_SLAVE_DEVICE_ADDR`` to the desired address used by the |I2C| master device.
  - ``appconfINTENT_I2C_REG_ADDRESS`` to the desired register read by the |I2C| master device.
  - ``appconfINTENT_I2C_MASTER_OUTPUT_ENABLED`` to 0, this will disable the |I2C| master interface after initialization.

The handling of the |I2C| slave registers is done in the ``examples\ffd\src\i2c_reg_handling.c`` file. The variable ``appconfINTENT_I2C_REG_ADDRESS`` is used in the callback function ``read_device_reg()``.

Configuring the |I2S| interface
-------------------------------

The |I2S| interface is used to send the intent audio to the DAC, and/or to receive the audio samples from the host. The |I2S| interface can be configured as either a master or a slave.
To configure the |I2S| interface, set the following variables:

  - ``appconfI2S_ENABLED`` to 1.
  - ``appconfI2S_MODE`` to the desired mode, either ``appconfI2S_MODE_MASTER`` or ``appconfI2S_MODE_SLAVE``.
  - ``appconfI2S_AUDIO_SAMPLE_RATE`` to the desired sample rate, either 16000 or 48000.
  - ``appconfRECOVER_MCLK_I2S_APP_PLL`` to 1 if an external MCLK is not available, otherwise set it to 0.
  - ``appconfAUDIO_PLAYBACK_ENABLED`` to 1, if the intent audio is to be played back.
  - ``appconfUSE_I2S_INPUT`` to 1, if the |I2S| audio source is to be used instead of the microphone array audio source.
