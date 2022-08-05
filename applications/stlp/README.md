# Far-field Voice Assistant

This is the far-field voice assistant reference design firmware.  See the full documentation for more information on configuring, modifying, building, and running the firmware.

## Supported Hardware

This example is supported on the XK_VOICE_L71 board.

## Building the Firmware

Run the following commands in the root folder to build the firmware.

On Linux and Mac run:

    cmake -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
    cd build

    make application_stlp_int_adec
    make application_stlp_int_adec_altarch
    make application_stlp_ua_adec
    make application_stlp_ua_adec_altarch

On Windows run:

    cmake -G "NMake Makefiles" -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
    cd build

    nmake application_stlp_int_adec
    nmake application_stlp_int_adec_altarch
    nmake application_stlp_ua_adec
    nmake application_stlp_ua_adec_altarch

From the build folder, create the filesystem and flash the device with the appropriate command to the desired configuration:

On Linux and Mac run:

    make flash_fs_application_stlp_int_adec
    make flash_fs_application_stlp_int_adec_altarch
    make flash_fs_application_stlp_ua_adec
    make flash_fs_application_stlp_ua_adec_altarch

On Windows run:

    nmake flash_fs_application_stlp_int_adec
    nmake flash_fs_application_stlp_int_adec_altarch
    nmake flash_fs_application_stlp_ua_adec
    nmake flash_fs_application_stlp_ua_adec_altarch

## Running the Firmware

Run the following commands in the build folder.

On Linux and Mac run:

    make run_application_stlp_int_adec
    make run_application_stlp_int_adec_altarch
    make run_application_stlp_ua_adec
    make run_application_stlp_ua_adec_altarch

On Windows run:

    nmake run_application_stlp_int_adec
    nmake run_application_stlp_int_adec_altarch
    nmake run_application_stlp_ua_adec
    nmake run_application_stlp_ua_adec_altarch

## Debugging the firmware with `xgdb`

Run the following commands in the build folder.

On Linux and Mac run:

    make debug_application_stlp_int_adec
    make debug_application_stlp_int_adec_altarch
    make debug_application_stlp_ua_adec
    make debug_application_stlp_ua_adec_altarch

On Windows run:

    nmake debug_application_stlp_int_adec
    nmake debug_application_stlp_int_adec_altarch
    nmake debug_application_stlp_ua_adec
    nmake debug_application_stlp_ua_adec_altarch

## Running the Firmware With WAV Files

This application supports USB audio input and output debug configuration.

To enable USB audio debug, configure cmake with:

Run the following commands in the root folder to build the firmware.

On Linux and Mac run:

    cmake -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake -DDEBUG_STLP_USB_MIC_INPUT=1
    cd build

    make application_stlp_ua_adec
    make application_stlp_ua_adec_altarch

On Windows run:

    cmake -G "NMake Makefiles" -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake -DDEBUG_STLP_USB_MIC_INPUT=1
    cd build

    nmake application_stlp_ua_adec
    nmake application_stlp_ua_adec_altarch

After rebuilding the firmware, run the application.

In a separate terminal, run the usb audio host utility provided in the tools/audio folder:

    process_wav.sh -c4 input.wav output.wav

This application requires the input audio wav file to be 4 channels in the order MIC 0, MIC 1, REF L, REF R.  Output is ASR, ignore, REF L, REF R, MIC 0, MIC 1, where the reference and microphone are passthrough.
