============================
Audio Mux Example Design
============================

This is the XMOS audio multiplexer example design.

This example application can be configured for onboard mic, usb audio, or i2s input.  Outputs are usb audio and i2s.  No DSP is performed on the audio, but the example contains an empty 2 tile pipeline skeleton for a user to populate.

******************
Supported Hardware
******************

This example is supported on the XCORE-AI-EXPLORER board.

******************
Host Setup
******************

On Linux/Mac the user may need to update their udev rules for USB configurations.  Add a custom udev rule for USB device with VID 0x20B1 and PID 0x0021.

*********************
Building the Firmware
*********************

Run the following commands in the root folder to build the firmware:

.. tab:: Linux and Mac

    .. code-block:: console

        cmake -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
        cd build
        make example_audio_mux

.. tab:: Windows

    .. code-block:: console

        cmake -G "NMake Makefiles" -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
        cd build
        nmake example_audio_mux


********************
Running the Firmware
********************

From the build folder run:

.. tab:: Linux and Mac

    .. code-block:: console

        make run_example_audio_mux

.. tab:: Windows

    .. code-block:: console

        nmake run_example_audio_mux


********************************
Debugging the firmware with xgdb
********************************

From the build folder run:

.. tab:: Linux and Mac

    .. code-block:: console

        make debug_example_audio_mux

.. tab:: Windows

    .. code-block:: console

        nmake debug_example_audio_mux
