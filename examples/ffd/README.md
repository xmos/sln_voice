# Far-field Voice Local Command

This is the XCORE-VOICE far-field voice local control firmware with Wanson speech recognition. 

This software is an evaluation version only. It includes a mechanism that limits the maximum number of recognitions to 50. You can reset the counter to 0 by restarting or rebooting the application.

The Wanson speech recognition library is [Copyright 2022. Shanghai
Wanson Electronic Technology Co.Ltd (&quot;WANSON&quot;)]
and is subject to the [Wanson Restrictive
License](https://github.com/xmos/sln_voice/blob/develop/examples/ffd/asr/port/wanson/lib/LICENSE.md).

See the full documentation for more information on configuring, modifying, building, and running the firmware.

## Speech Recognition

The application will recognize the following utterances:

**Wakeword Utterances**
- Hello XMOS

**Command Utterances**
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

## Supported Hardware

This example is supported on the XK_VOICE_L71 board.

## Building the Host Applications

This application requires a host application to create the flash data partition. Run the following commands in the root folder to build the host application using your native Toolchain:

NOTE: Permissions may be required to install the host applications.

On Linux and Mac run:

    cmake -B build_host
    cd build_host
    make install

The host applications will be installed at ``/opt/xmos/bin``, and may be moved if desired.  You may wish to add this directory to your ``PATH`` variable.

On Windows run:

Before building the host application, you will need to add the path to the XTC Tools to your environment.

  set "XMOS_TOOL_PATH=<path-to-xtc-tools>"

Then build the host application:

    cmake -G "NMake Makefiles" -B build_host
    cd build_host
    nmake install

The host applications will be install at ``<USERPROFILE>\.xmos\bin``, and may be moved if desired.  You may wish to add this directory to your ``PATH`` variable.

## Building the Firmware

Run the following commands in the root folder to build the firmware:

On Linux and Mac run:

    cmake -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
    cd build
    make example_ffd

On Windows run:

    cmake -G "NMake Makefiles" -B build -D CMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
    cd build
    nmake example_ffd

## Running the Firmware

Before the firmware is run, the data partition containing the filesystem and
model(s) must be loaded. Run the following commands from the build folder.

On Linux and Mac run:

    make flash_app_example_ffd

On Windows run:

    nmake flash_app_example_ffd

Once flashed, the application will run.

If changes are made to the data partition components, the application must be
re-flashed.

If there are no changes to the data partition, run the following from the build
folder.

On Linux and Mac run:

    make run_example_ffd

On Windows run:

    nmake run_example_ffd

## Debugging the firmware with `xgdb`

Run the following commands in the build folder.

On Linux and Mac run:

    make debug_example_ffd

On Windows run:

    nmake debug_example_ffd
