.. include:: ../../../substitutions.rst

.. _sln_voice_ffd_configuration:

Configuring the Firmware
========================

The default application performs as described in the :ref:`sln_voice_ffd_overview`. There are numerous compile time options that can be added to change the example design without requiring code changes.  To change the options explained in the table below, add the desired configuration variables to the APP_COMPILE_DEFINITIONS cmake variable located `here <https://github.com/xmos/sln_voice/blob/develop/examples/ffd/ffd.cmake>`_.

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
   * - appconfINTENT_I2C_OUTPUT_ENABLED
     - Enables/disables the |I2C| intent message
     - 1
   * - appconfUART_BAUD_RATE
     - Sets the baud rate for the UART tx intent interface
     - 9600
   * - appconfINTENT_I2C_OUTPUT_DEVICE_ADDR
     - Sets the |I2C| slave address to transmit the intent to
     - 0x01
   * - appconfINTENT_TRANSPORT_DELAY_MS
     - Sets the delay between host wake up requested and |I2C| and UART keyword code transmission
     - 50
   * - appconfINTENT_QUEUE_LEN
     - Sets the maximum number of detected intents to hold while waiting for the host to wake up
     - 10
   * - appconfINTENT_WAKEUP_EDGE_TYPE
     - Sets the host wake up pin GPIO edge type.  0 for rising edge, 1 for falling edge
     - 0
   * - appconfLOW_POWER_ENABLED
     - Enables/disables low power feature
     - 1
   * - appconfLOW_POWER_SWITCH_CLK_DIV_ENABLE
     - Enables/disables low power feature adjusting the switch clock
     - 1
   * - appconfLOW_POWER_SWITCH_CLK_DIV
     - Sets the low power mode switch clock divider value
     - 30
   * - appconfLOW_POWER_OTHER_TILE_CLK_DIV
     - Sets the low power mode tile core clock divider value for the keyword engine tile
     - 600
   * - appconfLOW_POWER_CONTROL_TILE_CLK_DIV
     - Sets the low power mode tile core clock divider value for the audio pipeline and voice activity detection tile
     - 2
   * - appconfPOWER_FULL_HOLD_DURATION
     - Sets the minimum amount of time to expect a wakeword before requesting to be set back into low power
     - 1000
   * - appconfAUDIO_PIPELINE_BUFFER_ENABLED
     - Enables/disables a ring buffer to hold pre-trigger audio frames when in low power
     - 1
   * - appconfAUDIO_PIPELINE_BUFFER_NUM_FRAMES
     - Sets the number of audio frames held in the low power audio frame buffer
     - 32
   * - appconfAUDIO_PIPELINE_SKIP_IC_AND_VNR
     - Enables/disables the IC and VNR
     - 0
   * - appconfAUDIO_PIPELINE_SKIP_NS
     - Enables/disables the NS
     - 0
   * - appconfAUDIO_PIPELINE_SKIP_AGC
     - Enables/disables the AGC
     - 0
