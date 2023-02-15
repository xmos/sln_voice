# Far-field Voice Assistant

This is the far-field voice assistant example design firmware.  See the full documentation for more information on configuring, modifying, building, and running the firmware.

## Supported Hardware

This example is supported on the XK_VOICE_L71 board.

## Building the Firmware

Run the following commands in the root folder to build the firmware.

On Linux and Mac run:

    cmake -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
    cd build

    make example_ffva_int_fixed_delay
    make example_ffva_ua_adec

On Windows run:

    cmake -G "NMake Makefiles" -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
    cd build

    nmake example_ffva_int_fixed_delay
    nmake example_ffva_ua_adec

From the build folder, create the data partition containing the filesystem and
flash the device with the appropriate command to the desired configuration:

On Linux and Mac run:

    make flash_app_example_ffva_int_fixed_delay
    make flash_app_example_ffva_ua_adec

On Windows run:

    nmake flash_app_example_ffva_int_fixed_delay
    nmake flash_app_example_ffva_ua_adec

Once flashed, the application will run.

If changes are made to the data partition components, the application must be
re-flashed.

## Running the Firmware

Run the following commands in the build folder.

On Linux and Mac run:

    make run_example_ffva_int_fixed_delay
    make run_example_ffva_ua_adec

On Windows run:

    nmake run_example_ffva_int_fixed_delay
    nmake run_example_ffva_ua_adec

## Debugging the firmware with `xgdb`

Run the following commands in the build folder.

On Linux and Mac run:

    make debug_example_ffva_int_fixed_delay
    make debug_example_ffva_ua_adec

On Windows run:

    nmake debug_example_ffva_int_fixed_delay
    nmake debug_example_ffva_ua_adec

## Running the Firmware With WAV Files

This application supports USB audio input and output debug configuration.

To enable USB audio debug, configure cmake with:

Run the following commands in the root folder to build the firmware.

On Linux and Mac run:

    cmake -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake -DDEBUG_FFVA_USB_MIC_INPUT=1
    cd build

    make example_ffva_ua_adec

On Windows run:

    cmake -G "NMake Makefiles" -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake -DDEBUG_FFVA_USB_MIC_INPUT=1
    cd build

    nmake example_ffva_ua_adec

After rebuilding the firmware, run the application.

In a separate terminal, run the usb audio host utility provided in the tools/audio folder:

    process_wav.sh -c4 input.wav output.wav

This application requires the input audio wav file to be 4 channels in the order MIC 0, MIC 1, REF L, REF R.  Output is ASR, ignore, REF L, REF R, MIC 0, MIC 1, where the reference and microphone are passthrough.
