# ASRC demo application

This is the ASRC demo example design firmware.  See the full documentation for more information on configuring, modifying, building, and running the firmware.

## Supported Hardware

This example is supported on the XK_VOICE_L71 board.


## Building the Firmware

Run the following commands in the root folder to build the firmware.

On Linux and Mac run:

    cmake -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
    cd build

    make example_asrc_demo

On Windows run:

    cmake -G Ninja -B build --toolchain=xmos_cmake_toolchain/xs3a.cmake

    ninja -C build example_asrc_demo

From the build folder, flash the device with the appropriate command to the desired configuration:

On Linux and Mac run:

    make flash_app_example_asrc_demo

On Windows run:

    nmake flash_app_example_asrc_demo

Once flashed, the application will run.


## Running the Firmware

Run the following commands in the build folder.

On Linux and Mac run:

    make run_example_example_asrc_demo

On Windows run:

    nmake run_example_example_asrc_demo

When running, the FW presents an I2S slave and a USB interface. To test, connect to an I2S master by connecting the BCLK, LRCK, DOUT and DIN pins on the XK_VOICE_L71 to the master. Audio streamed over the L71's USB interface is seen on the I2S data out interface and audio streamed into the L71's I2S data in interface is seen streamed out of the USB interface of the L71.
