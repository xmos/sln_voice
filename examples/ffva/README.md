# Far-field Voice Assistant

This is the far-field voice assistant example design firmware.  See the full documentation for more information on configuring, modifying, building, and running the firmware.

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

The host applications will be installed at ``%USERPROFILE%\.xmos\bin``, and may be moved if desired.  You may wish to add this directory to your ``PATH`` variable.

## Building the Firmware

Run the following commands in the root folder to build the firmware.

On Linux and Mac run:

    cmake -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
    cd build

    make example_ffva_int_fixed_delay
    make example_ffva_ua_adec_altarch

On Windows run:

    cmake -G "NMake Makefiles" -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
    cd build

    nmake example_ffva_int_fixed_delay
    nmake example_ffva_ua_adec_altarch

From the build folder, create the data partition containing the filesystem and
flash the device with the appropriate command to the desired configuration:

On Linux and Mac run:

    make flash_app_example_ffva_int_fixed_delay
    make flash_app_example_ffva_ua_adec_altarch

On Windows run:

    nmake flash_app_example_ffva_int_fixed_delay
    nmake flash_app_example_ffva_ua_adec_altarch

Once flashed, the application will run.

If changes are made to the data partition components, the application must be
re-flashed.

## Running the Firmware

Run the following commands in the build folder.

On Linux and Mac run:

    make run_example_ffva_int_fixed_delay
    make run_example_ffva_ua_adec_altarch

On Windows run:

    nmake run_example_ffva_int_fixed_delay
    nmake run_example_ffva_ua_adec_altarch

## Debugging the firmware with `xgdb`

Run the following commands in the build folder.

On Linux and Mac run:

    make debug_example_ffva_int_fixed_delay
    make debug_example_ffva_ua_adec_altarch

On Windows run:

    nmake debug_example_ffva_int_fixed_delay
    nmake debug_example_ffva_ua_adec_altarch

## Running the Firmware With WAV Files

This application supports USB audio input and output debug configuration.

To enable USB audio debug, configure cmake with:

Run the following commands in the root folder to build the firmware.

On Linux and Mac run:

    cmake -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake -DDEBUG_FFVA_USB_MIC_INPUT=1
    cd build

    make example_ffva_ua_adec_altarch

On Windows run:

    cmake -G "NMake Makefiles" -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake -DDEBUG_FFVA_USB_MIC_INPUT=1
    cd build

    nmake example_ffva_ua_adec_altarch

After rebuilding the firmware, run the application.

In a separate terminal, run the usb audio host utility provided in the tools/audio folder:

    process_wav.sh -c4 input.wav output.wav

This application requires the input audio wav file to be 4 channels in the order MIC 0, MIC 1, REF L, REF R.  Output is ASR, ignore, REF L, REF R, MIC 0, MIC 1, where the reference and microphone are passthrough.
