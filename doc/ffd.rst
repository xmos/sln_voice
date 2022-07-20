.. _sln_voice_FFD:

#############################
Far-field Voice Local Control
#############################

.. include:: <isonum.txt>

.. toctree::
   :maxdepth: 1
   :hidden:

   wanson
   ffd_host_integration
   ffd_modifying_software
   ffd_pipeline


Overview
========
This is the XMOS far-field local dictionary (FFD) reference design with Wanson speech recognition.

When a wakeup phrase is followed by an intent phrase the application will output an audio response, i2c and uart discrete message, and display text on the optional SSD1306 daughter board.

This software is an evaluation version only.  It includes a mechanism that limits the maximum number of recognitions to 50. You can reset the counter to 0 by restarting or rebooting the application.  The application can be rebooted by power cycling or pressing the SW2 button.

More information on the Wanson speech recognition library can be found here:

:ref:`sln_voice_Wanson`


Try it
===============

Supported Hardware
------------------

This reference application is supported on the `XK-VOICE-L71 <https://www.digikey.co.uk/en/products/detail/xmos/XK-VOICE-L71/15761172>`_ board.

Setting up the Hardware
-----------------------

xTAG
^^^^
The xTAG is used to program and debug the device

Connect the xTAG to the debug header, as shown below.
TODO: Image of how to connect xtag

Speakers (OPTIONAL)
^^^^^^^^^^^^^^^^^^^
This reference application features audio playback responses.  Speakers can be connected, as shown below.
TODO: Image of where to connect speakers

SSD1306 Display (OPTIONAL)
^^^^^^^^^^^^^^^^^^^^^^^^^^
Attach optional display daughter board, as shown below.
TODO: Image of setting up daughter board

Attach the SSD1306 display to the daughter board, as shown below.
TODO: Image of setting up SSD1306 display

Building the Firmware
---------------------

Run the following commands in the root folder to build the firmware:

.. tab:: Linux and Mac

    .. code-block:: console

        cmake -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
        cd build
        make application_ffd

.. tab:: Windows

    .. code-block:: console

        cmake -G "NMake Makefiles" -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
        cd build
        nmake application_ffd

Configuring the Firmware
------------------------

The default application performs as described in the Overview. There are numerous compile time options that can be added to change the reference design without requiring code changes.  To change the options explained in the table below, add the desired configuration variables to the APP_COMPILE_DEFINITIONS cmake variable located `here <https://github.com/xmos/sln_voice/blob/develop/applications/ffd/ffd.cmake>`_.

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
     - Enables/disables the I2C intent message
     - 1
   * - appconfUART_BAUD_RATE
     - Sets the baud rate for the UART tx intent interface
     - 9600
   * - appconfINFERENCE_I2C_OUTPUT_DEVICE_ADDR
     - Sets the I2C slave address to transmit the intent to
     - 0x01
   * - appconfINTENT_TRANSPORT_DELAY_MS
     - Sets the delay between host wake up requested and I2C and UART keyword code transmission
     - 50
   * - appconfINTENT_QUEUE_LEN
     - Sets the maximum number of detected intents to hold while waiting for the host to wake up
     - 10
   * - appconfINTENT_WAKEUP_EDGE_TYPE
     - Sets the host wake up pin GPIO edge type.  0 for rising edge, 1 for falling edge
     - 0
   * - appconfAUDIO_PIPELINE_SKIP_IC_AND_VAD
     - Enables/disables the IC and VAD
     - 0
   * - appconfAUDIO_PIPELINE_SKIP_NS
     - Enables/disables the NS
     - 0
   * - appconfAUDIO_PIPELINE_SKIP_AGC
     - Enables/disables the AGC
     - 0

Running the Firmware
--------------------

Before the firmware is run, the swmem and filesystem must be loaded.

Inside of the build folder root, after building the firmware:

.. tab:: Linux and Mac

    .. code-block:: console

        make flash_fs_application_ffd

.. tab:: Windows

    .. code-block:: console

        nmake flash_fs_application_ffd

Once flashed, the application will run.

After the filesystem and swmem have been flashed once, the application can be run without flashing.  If changes are made to the swmem or filesystem image, the application must be reflashed.

From the build folder run:

.. tab:: Linux and Mac

    .. code-block:: console

        make run_application_ffd

.. tab:: Windows

    .. code-block:: console

        nmake run_application_ffd

Debugging the Firmware
----------------------

To debug with xgdb, from the build folder run:

.. tab:: Linux and Mac

    .. code-block:: console

        make debug_application_ffd

.. tab:: Windows

    .. code-block:: console

        nmake debug_application_ffd


Host Integration
===================

This reference application can be integrated with existing solutions or modified to be a single controller solution.

Out of the Box Integration
--------------------------

In depth information on out of the box integration can be found here:
:ref:`sln_voice_ffd_host_integration`

.. |ffd_host_integration_diagram_image| figure:: diagrams/ffd_host_integration_diagram.drawio.png
   :align: center
   :scale: 80 %
   :alt: ffd host integration diagram

Single Controller Solution
--------------------------

In depth information on the software implementation for users to familiarize themselves with the application before modifying it can be found here:
:ref:`sln_voice_ffd_modifying_software`

.. |ffd_direct_control_diagram_image| figure:: diagrams/ffd_direct_control_diagram.drawio.png
   :align: center
   :scale: 80 %
   :alt: ffd host direct control diagram


Design Architecture
===================

This application span both tiles, consuming x memory on tile 0, and x memory on tile 1.

The application consists of a PDM microphone input, which is fed through the XMOS-VOICE DSP blocks.  The output ASR channel is then sent to the Wanson keyword engine.  The intent result is then handled, with discrete messaging to various IO interfaces and audio playback.

.. |ffd_diagram_image| figure:: diagrams/ffd_diagram.drawio.png
   :align: center
   :scale: 80 %
   :alt: ffd diagram

Audio Pipeline
==============

Information on the audio pipeline used by this application can be found here:
:ref:`sln_voice_ffd_ap`

Speech Recognition
==================

Information on the speech recognition library used by this application can be found here:
:ref:`sln_voice_Wanson`
