*****************************
Far-field Voice Local Command
*****************************

This is the XCORE-VOICE far-field voice local control firmware. Two examples are provided: one example uses the Sensory TrulyHandsfree™ (THF) speech recognition, and the other one uses the Cyberon DSPotter™ speech recognition.

This software is an evaluation version only. It includes a mechanism that limits the maximum number of recognitions. You can reset the counter to 0 by restarting or rebooting the application.

The Sensory TrulyHandsfree™ speech recognition library is `Copyright (C) 1995-2022 Sensory Inc.` and is provided as an expiring development license. Commercial licensing is granted by `Sensory Inc <https://www.sensory.com/>`_.

The Cyberon DSPotter™ speech recognition library is provided as an expiring development license. Commercial licensing is granted by `Cyberon Corporation <https://www.cyberon.com.tw/>`_ and is subject to deployment only on part number XU316-1024-QF60B-C24-CY.

See the full documentation for more information on configuring, modifying, building, and running the firmware.

Speech Recognition
******************

The application will recognize the following utterances:

Wakeword Utterances
-------------------

- Hello XMOS
- Hello Cyberon (DSpotter™ model only)

Command Utterances
------------------

- Switch on the TV
- Channel up
- Channel down
- Volume up
- Volume down
- Switch off the TV
- Switch on the lights
- Brightness up
- Brightness down
- Switch off the lights
- Switch on the fan
- Speed up the fan
- Slow down the fan
- Set higher temperature
- Set lower temperature
- Switch off the fan

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

Building the Host Applications
==============================

This application requires a host application to create the flash data partition. Run the following commands in the root folder to build the host application using your native Toolchain:

.. note::

    In the commands below ``<speech_engine>`` can be either ``sensory`` or ``cyberon``, depending on the choice of the speech recognition engine and model.

.. note::

    Permissions may be required to install the host applications.

On Linux or Mac run:

::

    cmake -B build_host
    cd build_host
    make install

The host applications will be installed at ``/opt/xmos/bin``, and may be moved if desired.  You may wish to add this directory to your ``PATH`` variable.

On Windows run:

::

    cmake -G Ninja -B build_host
    cd build_host
    ninja install

The host applications will be installed at ``%USERPROFILE%\.xmos\bin``, and may be moved if desired.  You may wish to add this directory to your ``PATH`` variable.

Building the Firmware
=====================

Run the following commands in the root folder to build the firmware:

On Linux and Mac run:

::

    cmake -B build --toolchain xmos_cmake_toolchain/xs3a.cmake
    cd build
    make example_ffd_<speech_engine>

On Windows run:

::

    cmake -G Ninja -B build --toolchain xmos_cmake_toolchain/xs3a.cmake
    cd build
    ninja example_ffd_<speech_engine>

Running the Firmware
====================

Before the firmware is run, the data partition containing the filesystem and
model(s) must be loaded. Run the following commands from the build folder.

On Linux and Mac run:

::

    make flash_app_example_ffd_<speech_engine>

On Windows run:

::

    ninja flash_app_example_ffd_<speech_engine>

Once flashed, the application will run.

If changes are made to the data partition components, the application must be
re-flashed.

If there are no changes to the data partition, run the following from the build
folder:

::

    xrun --xscope example_ffd_<speech_engine>.xe


Debugging the firmware with `xgdb`
=================================

Run the following commands in the build folder:

::

    xgdb -ex "connect --xscope" -ex "run" example_ffd_<speech_engine>.xe
