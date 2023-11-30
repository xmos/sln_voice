INTRODUCTION
============

This folder contains a simulation framework implementation of the ASRC demo application.
There are 2 simulation applications, the i2s_in_usb_out application that simulates the I2S -> ASRC -> USB direction and the
usb_in_i2s_out application that simulates the USB -> ASRC -> I2S direction.

REQUIREMENTS
============
The simulation framework requires a Linux or MacOS platform and the instructions below for building and running it assume so.

The simulation framework is implemented in C++ and uses the systemC and numCpp packages. The ASRC implementation used in the simulation
is the ASRC C emulator that is part of lib_src. Only mono channel ASRC is supported. All 3 dependencies (SystemC, numCpp and lib_src) are fetched as FetchContent during
the cmake process.

BUILDING
========

To build, do the following:
cmake -S . -B ./build

For building both applications
cmake --build build

For building i2s_in_usb_out application,
cmake --build build --target i2s_in_usb_out

For building usb_in_i2s_out application,
cmake --build build --target usb_in_i2s_out


RUNNING
=======

To run the applications, from this directory, run:

./build/i2s_in_usb_out <i2s_rate> <optional timestamps file>
./build/usb_in_i2s_out <i2s_rate> <optional timestamps file>

The <i2s_rate> argument is compulsory and can be one of the supported i2s rates, which are 192000, 176400, 96000, 88200, 48000 and 44100
Additionally, the user can provide an optional argument which is a file containing timestamps at which SOFs are received, to emulate the real system behaviour.
The file log_sofs_1hr is an example of this. It contains the xcore reference timer timestamps of SOF receive events recorded when running the actual ASRC demo
application on HW.
If the timestamps file is provided, the USB task is scheduled based on the timestamps instead of a fixed clock and this allows us to mimic the USB jitter seen when the device
is connected to a USB host.

RUNNING the i2s_in_usb_out application
======================================

When this application runs, it outputs values such as shown below on the stdout.
40,40
41,40
40,40
40,40
40,40
40,40

These numbers indicate the average buffer fill levels that are calculated every time there's a write to the buffer.
The numbers are printed as <long term average buffer fill level>,<short term average buffer fill level> on every line.
Once the simulation completes they can be plotted using the plot_csv script. For example,

From the current directory
./build/i2s_in_usb_out 96000 log_sofs_1hr 2>&1 > log
python python/plot_csv.py log 2 -p test.png -s

test.png contains plots for both columns of numbers present in the stdout log when i2s_in_usb_out is run.

RUNNING the usb_in_i2s_out application
======================================

Similar to the i2s_in_usb_out, when the usb_in_i2s_out application is run, it also outputs buffer fill level numbers on
the screen. Each line of the stdout is of the form <current buffer fill level>,<average buffer fill level>
The same script plot_csv.py can be used to plot this as well. For example,

From the current directory,
./build/usb_in_i2s_out 96000 log_sofs_1hr 2>&1 > log
python python/plot_csv.py log 2 -p test.png -s

ASRC INPUT and OUTPUT
=====================

When the simulation runs, a sine tone is generated at run time (using the numCpp library) and used as the input to the ASRC. This sine tone is also
dumped into the binary file asrc_input.bin. The ASRC output is also dumped to another binary file, asrc_output.bin.

The ASRC input and the output can be plotted in the frequency domain and their SNR calculated using the python/calc_snr.py script.

From the current directory,
python python/calc_snr.py asrc_input.bin <ASRC input rate>
python python/calc_snr.py asrc_output.bin <ASRC output rate>

For example, if running the usb_in_i2s_out application with I2S rate set to 192000
./usb_in_i2s_out 192000 ../log_sofs_1hr

The ASRC input rate would be the USB rate of 48000 and the ASRC output rate would be the I2S rate of 192000, so we could then run
python python/calc_snr.py asrc_input.bin 48000
python python/calc_snr.py asrc_output.bin 192000
