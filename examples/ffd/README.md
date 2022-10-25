# Far-field Voice Local Control

This is the XCORE-VOICE far-field voice local control firmware with Wanson speech recognition. 

This software is an evaluation version only. It includes a mechanism that limits the maximum number of recognitions to 50. You can reset the counter to 0 by restarting or rebooting the application.

The Wanson speech recognition library is [Copyright 2022. Shanghai
Wanson Electronic Technology Co.Ltd (&quot;WANSON&quot;)]{.title-ref}
and is subject to the [Wanson Restrictive
License](https://github.com/xmos/sln_voice/tree/develop/examples/ffd/inference/wanson/lib/LICENSE.md).

See the full documentation for more information on configuring, modifying, building, and running the firmware.

## Speech Recognition

The application will recognize the following utterances:

**Wakeword Utterances**
- Hello XMOS
- Hello Wanson

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

Before the firmware is run, the filesystem must be loaded. Run the following commands in the build folder.

On Linux and Mac run:

    make flash_fs_example_ffd

On Windows run:

    nmake flash_fs_example_ffd

Once flashed, the example will run.

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
