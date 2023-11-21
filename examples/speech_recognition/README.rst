************************************
Automated Speech Recognition Porting
************************************

This is the XCORE-VOICE automated speech recognition (ASR) porting example application.

Supported Hardware and pre-requisites
=====================================

This example is supported on the XK_VOICE_L71 board.  However, the XCORE-AI-EXPLORER board can be supported with a couple minor modifications.

On the host machine the XTC tools, version 15.2.1, must be installed and sourced.
The output should be
something like this:

::

   $ xcc --version
   xcc: Build 19-198606c, Oct-25-2022
   XTC version: 15.2.1
   Copyright (C) XMOS Limited 2008-2021. All Rights Reserved.

On Windows it is highly recommended to use ``Ninja`` as the make system under
``cmake``. Not only is it a lot faster than MSVC ``nmake``, it also
works around an issue where certain path names may cause an issue with
the XMOS compiler under Windows.

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

Building the host server
========================

This application requires a host application to serve files to the device. The served file must be named `test.wav`.  This filename is defined in `src/app_conf.h`.

Run the following commands in the root folder to build the host application using your native x86 Toolchain:

**Permissions may be required to install the host applications.**

Linux or Mac
------------

::

    cmake -B build_host
    cd build_host
    make xscope_host_endpoint
    make install

The host application, `xscope_host_endpoint`, will be installed at `/opt/xmos/bin/`, and may be moved if desired.  You may wish to add this directory to your `PATH` variable.

Before running the host application, you may need to add the location of `xscope_endpoint.so` to your `LD_LIBRARY_PATH` environment variable.  This environment variable will be set if you run the host application in the XTC Tools command-line environment.  For more information see `Configuring the command-line environment <https://www.xmos.ai/documentation/XM-014363-PC-LATEST/html/tools-guide/install-configure/getting-started.html>`__.

Windows
-------

It is highly recommended to use ``Ninja`` as the make system under
``cmake``. Not only is it a lot faster than MSVC ``nmake``, it also
works around an issue where certain path names may cause an issue with
the XMOS compiler under windows.

To install Ninja, follow these steps:

-  Download ``ninja.exe`` from
   https://github.com/ninja-build/ninja/releases. This firmware has been
   tested with Ninja version v1.11.1.
-  Ensure Ninja is on the command line path. You can add to the path
   permanently by following these steps
   https://www.computerhope.com/issues/ch000549.htm. Alternatively you
   may set the path in the current command line session using something
   like ``set PATH=%PATH%;C:\Users\xmos\utils\ninja``

Before building the host application, you will need to add the path to the XTC Tools to your environment:

::

    set "XMOS_TOOL_PATH=<path-to-xtc-tools>"

Then build the host application:

::

    cmake -G Ninja -B build_host
    cd build_host
    ninja xscope_host_endpoint
    ninja install

The host application, `xscope_host_endpoint.exe`, will install at `<USERPROFILE>\.xmos\bin`, and may be moved if desired.  You may wish to add this directory to your `PATH` variable.

Before running the host application, you may need to add the location of `xscope_endpoint.dll` to your `PATH`. This environment variable will be set if you run the host application in the XTC Tools command-line environment.  For more information see `Configuring the command-line environment <https://www.xmos.ai/documentation/XM-014363-PC-LATEST/html/tools-guide/install-configure/getting-started.html>`__.

Building the Firmware
=====================

Run the following commands in the root folder to build the firmware:

On Linux and Mac run:

::

    cmake -B build --toolchain xmos_cmake_toolchain/xs3a.cmake
    cd build
    make example_asr

On Windows run:

::

    cmake -G "Ninja" -B build --toolchain xmos_cmake_toolchain/xs3a.cmake
    cd build
    ninja example_asr

Flashing the Model
==================

Run the following commands in the build folder to flash the model:

::

    xflash --force --quad-spi-clock 50MHz --factory example_asr.xe --boot-partition-size 0x100000 --target-file ../examples/speech_recognition/XCORE-AI-EXPLORER.xn --data ../examples/speech_recognition/asr/port/example/asr_example_model.dat

Running the Firmware
====================

Run the following command in the build folder to run the firmware:

::

    xrun --xscope-realtime --xscope-port localhost:12345 example_asr.xe

In a second console, run the following command in the ``examples/speech_recognition`` folder to run the host server:

On Linux and Mac run:

::

    xscope_host_endpoint 12345

On Windows run:

::

    xscope_host_endpoint.exe 12345
