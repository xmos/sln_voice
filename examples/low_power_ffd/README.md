# Low-Power Far-field Voice Local Command

This is the XCORE-VOICE low-power far-field voice local control firmware with Sensory TrulyHandsFree speech recognition. 

Please note that this software comes with an expiring Sensory development license.  It will suspend recognition after 11.4 hours or 107 recognition events. The only way to clear this is to reset the system.

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

    cmake -G Ninja -B build_host
    cd build_host
    ninja install

The host applications will be installed at ``%USERPROFILE%\.xmos\bin``, and may be moved if desired.  You may wish to add this directory to your ``PATH`` variable.

## Building the Firmware

Run the following commands in the root folder to build the firmware:

On Linux and Mac run:

    cmake -B build --toolchain xmos_cmake_toolchain/xs3a.cmake
    cd build
    make example_low_power_ffd

On Windows run:

    cmake -G Ninja -B build --toolchain xmos_cmake_toolchain/xs3a.cmake
    cd build
    ninja example_low_power_ffd

## Running the Firmware

Before the firmware is run, the data partition containing the filesystem and
model(s) must be loaded. Run the following commands from the build folder.

On Linux and Mac run:

    make flash_app_example_low_power_ffd

On Windows run:

    ninja flash_app_example_low_power_ffd

Once flashed, the application will run.

If changes are made to the data partition components, the application must be
re-flashed.

If there are no changes to the data partition, run the following from the build
folder.

On Linux and Mac run:

    make run_example_low_power_ffd

On Windows run:

    ninja run_example_low_power_ffd

## Debugging the firmware with `xgdb`

Run the following commands in the build folder.

On Linux and Mac run:

    make debug_example_low_power_ffd

On Windows run:

    ninja debug_example_low_power_ffd
