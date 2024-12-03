*************************
Far-field Voice Assistant
*************************

This is the far-field voice assistant example design firmware.  See the full documentation for more information on configuring, modifying, building, and running the firmware.

Supported Hardware and pre-requisites
=====================================

This example is supported on the XK_VOICE_L71 board.

On the host machine the XTC tools, version 15.2.1, must be installed and sourced.
The output should be
something like this:

::

   $ xcc --version
   xcc: Build 19-198606c, Oct-25-2022
   XTC version: 15.2.1
   Copyright (C) XMOS Limited 2008-2021. All Rights Reserved.

It is recommended to use `Ninja` or `xmake` as the make system under Windows.
`Ninja` has been observed to be faster than `xmake`, however `xmake` comes natively with XTC tools.
This firmware has been tested with `Ninja` version v1.11.1.

To install Ninja, follow these steps:

-  Download ``ninja.exe`` from
   https://github.com/ninja-build/ninja/releases. This firmware has been
   tested with Ninja version v1.11.1.
-  Ensure Ninja is on the command line path. It can be added to the path
   permanently by following these steps
   https://www.computerhope.com/issues/ch000549.htm. Alternatively,
   set the path in the current command line session using something
   like ``set PATH=%PATH%;C:\Users\xmos\utils\ninja``

Before building the host application, you will need to add the path to the XTC Tools to your environment.

  set "XMOS_TOOL_PATH=<path-to-xtc-tools>"

Building the Host Applications
==============================

This application requires a host application to create the flash data partition. Run the following commands in the root folder to build the host application using your native Toolchain:

.. note::

    Permissions may be required to install the host applications.

On Linux and Mac run:

    cmake -B build_host
    cd build_host
    make install

The host applications will be installed at ``/opt/xmos/bin``, and may be moved if desired.  You may wish to add this directory to your ``PATH`` variable.

On Windows run:

Before building the host application, you will need to add the path to the XTC Tools to your environment:

::

    set "XMOS_TOOL_PATH=<path-to-xtc-tools>"

Then build the host application:

::

    cmake -G Ninja -B build_host
    cd build_host
    ninja install

The host applications will be installed at ``%USERPROFILE%\.xmos\bin``, and may be moved if desired.  You may wish to add this directory to your ``PATH`` variable.

Building the Firmware
=====================

After having your python environment activated, run the following commands in the root folder to build the firmware:

On Linux and Mac run:

::

    pip install -r requirements.txt
    cmake -B build --toolchain xmos_cmake_toolchain/xs3a.cmake
    cd build

    make example_ffva_ua_adec_altarch
    make example_ffva_int_fixed_delay
    make example_ffva_int_cyberon_fixed_delay

On Windows run:

::

    pip install -r requirements.txt
    cmake -G Ninja -B build --toolchain xmos_cmake_toolchain/xs3a.cmake
    cd build

    ninja example_ffva_ua_adec_altarch
    ninja example_ffva_int_fixed_delay
    ninja example_ffva_int_cyberon_fixed_delay

From the build folder, create the data partition containing the filesystem and
flash the device with the appropriate command to the desired configuration:

On Linux and Mac run:

::

    make flash_app_example_ffva_ua_adec_altarch
    make flash_app_example_ffva_int_fixed_delay
    make flash_app_example_ffva_int_cyberon_fixed_delay

On Windows run:

::

    ninja flash_app_example_ffva_ua_adec_altarch
    ninja flash_app_example_ffva_int_fixed_delay
    ninja flash_app_example_ffva_int_cyberon_fixed_delay

Once flashed, the application will run.

If changes are made to the data partition components, the application must be
re-flashed.

Running the Firmware
====================

Run the following commands in the build folder:

::

    xrun --xscope example_ffva_ua_adec_altarch.xe
    xrun --xscope example_ffva_int_fixed_delay.xe
    xrun --xscope example_ffva_int_cyberon_fixed_delay.xe


Debugging the firmware with `xgdb`
=================================

Run the following commands in the build folder:

::

    xgdb -ex "conn --xscope" -ex "r" example_ffva_ua_adec_altarch.xe
    xgdb -ex "conn --xscope" -ex "r" example_ffva_int_cyberon_fixed_delay.xe


Running the Firmware With WAV Files
===================================

This application supports USB audio input and output debug configuration.

To enable USB audio debug, configure cmake with:

After having your python environment activated, run the following commands in the root folder to build the firmware:

On Linux and Mac run::

::

    pip install -r requirements.txt
    cmake -B build --toolchain xmos_cmake_toolchain/xs3a.cmake -DDEBUG_FFVA_USB_MIC_INPUT=1
    cd build

    make example_ffva_ua_adec_altarch

On Windows run:

::

    pip install -r requirements.txt
    cmake -G Ninja -B build --toolchain xmos_cmake_toolchain/xs3a.cmake -DDEBUG_FFVA_USB_MIC_INPUT=1
    cd build

    ninja example_ffva_ua_adec_altarch

After rebuilding the firmware, run the application.

In a separate terminal, run the usb audio host utility provided in the tools/audio folder:

::

    process_wav.sh -c4 input.wav output.wav

This application requires the input audio wav file to be 4 channels in the order MIC 0, MIC 1, REF L, REF R.  Output is ASR, ignore, REF L, REF R, MIC 0, MIC 1, where the reference and microphone are passthrough.
