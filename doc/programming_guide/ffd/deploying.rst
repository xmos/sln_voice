.. include:: ../../substitutions.rst

**********************
Deploying the Software
**********************

Building the Firmware
=====================

Run the following commands in the root folder to build the firmware:

**Linux and Mac**

.. code-block:: console

    cmake -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
    cd build
    make example_ffd

**Windows**

.. code-block:: console

    cmake -G "NMake Makefiles" -B build -D CMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
    cd build
    nmake example_ffd

Configuring the Firmware
========================

The default application performs as described in the Overview. There are numerous compile time options that can be added to change the example design without requiring code changes.  To change the options explained in the table below, add the desired configuration variables to the APP_COMPILE_DEFINITIONS cmake variable located `here <https://github.com/xmos/sln_voice/blob/develop/examples/ffd/ffd.cmake>`_.

If options are changed, the application firmware must be rebuilt.

.. list-table:: FFD Compile Options
   :widths: 50 100 50
   :header-rows: 1
   :align: left

   * - Compile Option
     - Description
     - Default Value
   * - appconfINFERENCE_ENABLED
     - Enables/disables the keyword spotter, primarily for debug.
     - 1
   * - appconfINFERENCE_RESET_DELAY_MS
     - Sets the period after the wake up phrase has been heard for a valid command phrase
     - 3000
   * - appconfINFERENCE_RAW_OUTPUT
     - Set to 1 to output all keywords found, skipping the internal wake up and command state machine
     - 0
   * - appconfAUDIO_PLAYBACK_ENABLED
     - Enables/disables the audio playback command response
     - 1
   * - appconfINFERENCE_UART_OUTPUT_ENABLED
     - Enables/disables the UART intent message
     - 1
   * - appconfSSD1306_DISPLAY_ENABLED
     - Enables/disables the SSD1306 daughter board display intent message
     - 1
   * - appconfINFERENCE_I2C_OUTPUT_ENABLED
     - Enables/disables the |I2C| intent message
     - 1
   * - appconfUART_BAUD_RATE
     - Sets the baud rate for the UART tx intent interface
     - 9600
   * - appconfINFERENCE_I2C_OUTPUT_DEVICE_ADDR
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
   * - appconfAUDIO_PIPELINE_SKIP_IC_AND_VNR
     - Enables/disables the IC and VNR
     - 0
   * - appconfAUDIO_PIPELINE_SKIP_NS
     - Enables/disables the NS
     - 0
   * - appconfAUDIO_PIPELINE_SKIP_AGC
     - Enables/disables the AGC
     - 0

Running the Firmware
====================

Before the firmware is run, the data partition containing the filesystem and
model(s) must be loaded. Run the following commands from the build folder.

**Linux and Mac**

.. code-block:: console

    make flash_app_example_ffd

**Windows**

.. code-block:: console

    nmake flash_app_example_ffd

Once flashed, the application will run.

If changes are made to the data partition components, the application must be
re-flashed.

If there are no changes to the data partition, run the following from the build
folder.

**Linux and Mac**

.. code-block:: console

    make run_example_ffd

**Windows**

.. code-block:: console

    nmake run_example_ffd

Debugging the Firmware
======================

To debug with xgdb, from the build folder run:

**Linux and Mac**

.. code-block:: console

    make debug_example_ffd

**Windows**

.. code-block:: console

    nmake debug_example_ffd
