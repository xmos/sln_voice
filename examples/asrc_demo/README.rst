*********************
ASRC Demo Application
*********************

.. warning::

   This example is based on the RTOS framework and drivers, and it can lead to latency of up to 20 ms in the system.
   More information can be found in the Overview section of the ASRC example in the Programming Guide.

   For a proposed implementation with lower latency, please refer to the bare-metal examples below:

      - `AN02003: SPDIF/ADAT/I2S Slave Receive to I2S Slave Bridge with ASRC <https://www.xmos.com/file/AN02003>`__


This is the XCORE-VOICE Asynchronous Sampling Rate Converter (ASRC) example design.

The example system implements a stereo I2S slave and a stereo adaptive UAC2.0 interface and exchanges data between the two interfaces.
Since the two interfaces are operating in different clock domains, there is an ASRC block between them that converts from the input to the output sampling rate.
There are two ASRC blocks, one in the I2S -> ASRC -> USB path and the other in the USB -> ASRC -> I2S path.

The application also monitors and computes the instantaneous ratio between the ASRC input and output sampling rate. The rate ratio is used by the ASRC task to dynamically adapt filter coefficients using spline interpolation in its filtering stage.


The I2S slave interface is a stereo 32 bit interface supporting sampling rates between 44.1 kHz - 192 kHz.

The USB interface is a stereo, 32 bit, 48 kHz, High-Speed, USB Audio Class 2, Adaptive interface.

The ASRC algorithm in the `lib_src <https://github.com/xmos/lib_src/>`_  library is used for the ASRC processing. The ASRC processing is block based and works on a block size of 244 samples per channel in the I2S -> ASRC -> USB path and 96 samples per channel in the USB -> ASRC -> I2S path.

Supported Hardware
==================

This example application is supported on the `XK-VOICE-L71 <https://www.digikey.co.uk/en/products/detail/xmos/XK-VOICE-L71/15761172>`_ board.
In addition to the XK-VOICE-L71 board, it requires an XTAG4 to program and debug the device.

To demonstrate the audio exchange between the I2S and USB interface, the XK-VOICE-L71 device needs to be connected to an I2S master device.
To do this, connect the BCLK, MCLK, DOUT, DIN pins of the RASPBERRY PI HOST INTERFACE header (J4) on the XK-VOICE-L71 to the I2S master.
The table below lists the pins on the XK-VOICE-L71 RPI header and the signals on the I2S master that they need to be connected to.

+------------------------+---------------------------------------+
| XK-VOICE-L71 PI        | Connect to                            |
| header pin             |                                       |
+========================+=======================================+
| 12                     | BLCK output on the I2S master board   |
+------------------------+---------------------------------------+
| 35                     | LRCK output on the I2S master board   |
+------------------------+---------------------------------------+
| 38                     | I2S Data IN on the I2S master board   |
+------------------------+---------------------------------------+
| 40                     | I2S Data OUT on the I2S master board  |
+------------------------+---------------------------------------+

In addition to the pins listed above, make sure to connect one of the GND pins, (6, 14, 20, 30, 34, 9, 25 or 39) on the XK-VOICE-L71 PI
header to the GND on the I2S master board.


Obtaining the app files
=======================

Download the main repo and submodules using:

::

   $ git clone --recurse git@github.com:xmos/sln_voice.git
   $ cd sln_voice/


Building the app
================

First make sure that your XTC tools environment is activated.

Linux or Mac
------------

After having your python environment activated, run the following commands in the root folder to build the firmware:

::
   $ pip install -r requirements.txt
   $ mkdir build
   $ cd build
   $ cmake --toolchain ../xmos_cmake_toolchain/xs3a.cmake  ..
   $ make example_asrc_demo -j

Following initial ``cmake`` build, for subsequent builds, as long as new source files are not added, just type:

::

   $ make example_asrc_demo -j

``cmake`` needs to be rerun to discover any new source files added.

Windows
-------

It is recommended to use `Ninja` or `xmake` as the make system under Windows.
`Ninja` has been observed to be faster than `xmake`, however `xmake` comes natively with XTC tools.
This firmware has been tested with `Ninja` version v1.11.1.

To install Ninja, activate your python environment, and run the following command:

::

   $ pip install ninja

After having your python environment activated, run the following commands in the root folder to build the firmware:

::

   $ pip install -r requirements.txt
   $ md build
   $ cd build
   $ cmake -G "Ninja" --toolchain  ..\xmos_cmake_toolchain\xs3a.cmake ..
   $ ninja example_asrc_demo.xe -j

Following initial ``cmake`` build, for subsequent builds, as long as new source files are not added, just type:

::

   $ ninja example_asrc_demo.xe -j

``cmake`` needs to be rerun to discover any new source files added.

Running the app
===============

To run the app, either xrun or xflash can be used. Connect the XK-VOICE-L71 board to the host and type:

::

   $ xrun example_asrc_demo.xe

Optionally, xrun ``--xscope`` can be used to provide debug output.

or

::

   $ xflash example_asrc_demo.xe



Operation
=========

When the example runs, the audio received by the device on the I2S slave interface at the I2S interface sampling rate is
sample rate converted using the ASRC to the USB sampling rate and streamed out from the device over the USB interface. Similarly,
the audio streamed out by the USB host into the USB interface of the device is sample rate converted to the I2S interface sampling
rate and streamed out from the device over the I2S slave interface.

This example supports dynamic changes of the I2S interface sampling frequency at runtime. It detects the I2S sampling rate change and reconfigures
the system for the new rate.
