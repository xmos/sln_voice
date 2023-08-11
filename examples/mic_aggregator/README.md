# mic_aggregator_demo
This repo contains an app that provides 16 PDM mics to either TDM16 slave or USB Audio demo running on the explorer board. It uses a modified mic_array with multiple threads to support 16 DDR mics on a single 8b input port.

The decimator is configured to 48kHz PCM output. The 16 channels are loaded into a 16 slot TDM slave running at 24.576MHz bit clock or a USB Audio Class 2 asynchronous interface and optionally amplified.

For the TDM build, a simple TDM16 master is included as well as a local 24.576MHz clock source so that mic_array and TDM16 slave may be tested standalone through the use of jumper cables. These may be removed when integrating into a system with TDM16 master supplied.

Obtaining the app files
=======================

Download the main repo and submodules using:

    $ git clone --recurse git@github.com:ed-xmos/mic_aggregator_demo.git
    $ cd mic_aggregator_demo/


Building the app
================

First install and source the XTC version: 15.2.1 tools. You should be able to see:

    $ xcc --version
    xcc: Build 19-198606c, Oct-25-2022
    XTC version: 15.2.1
    Copyright (C) XMOS Limited 2008-2021. All Rights Reserved.

Linux or Mac
------------

To build for the first time you will need to run cmake to create the make files:

    $ mkdir build
    $ cd build
    $ cmake --toolchain ../fwk_io/xmos_cmake_toolchain/xs3a.cmake  ..
    $ make mic_aggregator_tdm -j
    $ make mic_aggregator_usb -j

Following initial cmake build, as long as you don't add new source files, you may just type:

    $ make mic_aggregator_tdm -j
    $ make mic_aggregator_usb -j

If you add new source files you will need to run the `cmake` step again.

Windows
-------

It is highly recommended to use `Ninja` as the make system under cmake. Not only is it a lot faster
than MSVC `nmake`, it also works around an issue where certain path names may cause an issue with the XMOS compiler under windows.

To install Ninja, follow these steps:

- Download `ninja.exe` from https://github.com/ninja-build/ninja/releases. This firmware has been tested with Ninja version v1.11.1
- Ensure Ninja is on the command line path. You can add to the path permenantly by following these steps https://www.computerhope.com/issues/ch000549.htm. Alternatively you may set the path in the current command line session using something like `set PATH=%PATH%;C:\Users\xmos\utils\ninja`

To build for the first time you will need to run cmake to create the make files:

    $ md build
    $ cd build
    $ cmake -G "Ninja" --toolchain  ..\fwk_io\xmos_cmake_toolchain\xs3a.cmake ..
    $ ninja mic_aggregator_tdm.xe -j
    $ ninja mic_aggregator_usb.xe -j

Following inital cmake build, as long as you don't add new source files, you may just type:

    $ ninja mic_aggregator_tdm.xe -j
    $ ninja mic_aggregator_usb.xe -j

If you add new source files you will need to run the `cmake` step again.

Known Issues
============

If using USB, there is currently a bug where an allocated timer is NULL causing an ET_ILLEGAL_RESOURCE at runtime. To work around this
it is currently necessary to replace line 141 of `xud_device.xc` in `/fwk_io/modules/xud/lib_xud/lib_xud/src/user/control/`:

     t when timerafter(time+50000) :> void;

with this line:

    unsigned t_now = time; do{t :> t_now;}while timeafter(t_now, time+50000); 

Running the app
===============

Connect the explorer board to the host and type:

    $ xrun app_mic_aggregator/mic_aggregator_tdm.xe 
    $ xrun app_mic_aggregator/mic_aggregator_usb.xe 

Optionally, you may use xrun `--xscope` to provide debug output.

Required Hardware
=================

The demo runs on the XCORE-AI Explorer board version 2 (with integrated XTAG debug adapter). You will require in addition:

- The dual DDR microphone board that attaches via the flat flex connector
- Header pins soldered into
    - J14, J10, SCL/SDA IOT, the I2S expansion header, MIC data and MIC clock
- A handful of jumper wires connected as below

An oscilloscope will also be handy in case of debug needed.

*Note you will only be able to inject PDM data to two channels at a time due to a single pair of mics on the HW*

If you wish to see all 16 mics running then an external mic board with 16 mics (DDR connected to 8 data lines) is required.


Jumper Connections
==================

Make the following connections using flying leads:

- MIC CLK <-> J14 '00'. This is the mic clock which is to be sent to the PDM mics from J14.
- MIC DATA <-> J14 '14' initially. This is the data line for mics 0 and 8. See below..
- I2S LRCLK <-> J10 '36'. This is the FSYCNH input for TDM slave. J10 '36' is the TDM master FSYNCH output for the demo
- I2S MCLK <-> I2S BCLK. MCLK is the 24.576MHz clock which directly drives the BCLK input for the TDM slave
- I2S DAC <-> J10 '38'. I2S DAC is the TDM Slave Tx out which is read by the TDM Master Rx input on J10.

To access other mic inputs use the following:

| Mic pair | J14 pin |
| -------- | ------- |
| 0, 8 | 14 |
| 1, 9 | 15 |
| 2, 10 | 16 |
| 3, 11 | 17 |
| 4, 12 | 18 |
| 5, 13 | 19 |
| 6, 14 | 20 |
| 7, 15 | 21 |


For I2C control, make the following connections:

- SCL IOL <-> Your I2C host SCL
- SDA IOL <-> Your I2C host SDA
- GND <-> Your I2C host ground

The I2C slave is tested to 100kHz SCL.

There are 32 registers which control the gain of each of the 16 output channels. The 8b registers contain the 
upper 8b and lower 8b of the mic gain respectively. The initial gain is set to 100, since 1 is quiet due to the 
mic_array output being scaled to allow acoustic overload of the mics without clipping. Typically a gain of
a few hundred works for normal conditions. The gain is only applied after the lower byte is written.

The gain applied is saturating so no overflow will occur, only clipping.

| Register | Value |
| -------- | ------- |
| 0 | Channel 0 upper gain byte |
| 1 | Channel 0 lower gain byte |
| 2 | Channel 1 upper gain byte |
| 3 | Channel 1 lower gain byte |
| 4 | Channel 2 upper gain byte |
| 5 | Channel 2 lower gain byte |
| 6 | Channel 3 upper gain byte |
| 7 | Channel 3 lower gain byte |
| 8 | Channel 4 upper gain byte |
| 9 | Channel 4 lower gain byte |
| 10 | Channel 5 upper gain byte |
| 11 | Channel 5 lower gain byte |
| 12 | Channel 6 upper gain byte |
| 13 | Channel 6 lower gain byte |
| 14 | Channel 7 upper gain byte |
| 15 | Channel 7 lower gain byte |
| 16 | Channel 8 upper gain byte |
| 17 | Channel 8 lower gain byte |
| 18 | Channel 9 upper gain byte |
| 19 | Channel 9 lower gain byte |
| 20 | Channel 10 upper gain byte |
| 21 | Channel 10 lower gain byte |
| 22 | Channel 11 upper gain byte |
| 23 | Channel 11 lower gain byte |
| 24 | Channel 12 upper gain byte |
| 25 | Channel 12 lower gain byte |
| 26 | Channel 13 upper gain byte |
| 27 | Channel 13 lower gain byte |
| 28 | Channel 14 upper gain byte |
| 29 | Channel 14 lower gain byte |
| 30 | Channel 15 upper gain byte |
| 31 | Channel 15 lower gain byte |


If using a raspberry Pi as the I2C host you may use the following commands:

    $ i2cset -y 1 0x3c 0 0 #Set the gain on mic channel 0 to 50
    $ i2cset -y 1 0x3c 1 50 #Set the gain on mic channel 0 to 50

    $ i2cget -y 1 0x3c 0 #Get the upper byte of gain on mic channel 0
    $ i2cget -y 1 0x3c 1 #Get the lower byte of gain on mic channel 0

    $ i2cset -y 1 0x3c 16 1 #Set the gain on mic channel 8 to 256
    $ i2cset -y 1 0x3c 1 0 #Set the gain on mic channel 8 to 256



