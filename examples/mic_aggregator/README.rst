#################################
PDM Microphone Aggregator Example
#################################

.. warning::
   This example is deprecated and will be moved into a separate
   Application Note and may be removed in the next major release.

This example provides a bridge between 16 PDM microphones to either
TDM16 slave or USB Audio and targets the xcore-ai explorer board.

This application is to support cases where many microphone inputs need
to be sent to a host where signal processing will be performed. Please
see the other examples in sln_voice where signal processing is performed
within the xcore in firmware.

This example uses a modified mic_array with multiple decimator threads to
support 16 DDR microphones on a single 8 bit input port. The example is written as
‘bare-metal’ and runs directly on the XCORE device without an RTOS.


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
   $ make example_mic_aggregator_tdm -j
   $ make example_mic_aggregator_usb -j

Following initial ``cmake`` build, as long as you don’t add new source
files, you may just type:

::

   $ make example_mic_aggregator_tdm -j
   $ make example_mic_aggregator_usb -j

If you add new source files you will need to run the ``cmake`` step
again.

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
   $ ninja example_mic_aggregator_tdm.xe -j
   $ ninja example_mic_aggregator_usb.xe -j

Following initial ``cmake`` build, as long as you don’t add new source
files, you may just type:

::

   $ ninja example_mic_aggregator_tdm.xe -j
   $ ninja example_mic_aggregator_usb.xe -j

If you add new source files you will need to run the ``cmake`` step
again.

Running the app
===============

Connect the explorer board to the host and type:

::

   $ xrun example_mic_aggregator_tdm.xe
   $ xrun example_mic_aggregator_usb.xe

Optionally, you may use xrun ``--xscope`` to provide debug output.

Required Hardware
=================

The application runs on the XCORE-AI Explorer board version 2 (with
integrated XTAG debug adapter). You will require in addition:

-  The dual DDR microphone board that attaches via the flat flex
   connector.
-  Header pins soldered into:

   -  J14, J10, SCL/SDA IOT, the I2S expansion header, MIC data and MIC
      clock.

-  Six jumper wires. Please see the microphone aggregator main documentation
   for details on how these are connected.

An oscilloscope will also be handy in case of hardware debug being needed.


.. note::

    You will only be able to inject PDM data to two channels at a time due to a single pair of microphones on the HW.


If you wish to see all 16 microphones running then an external microphone board
with 16 microphones (DDR connected to 8 data lines) is required.
