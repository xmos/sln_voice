.. include:: ../../../substitutions.rst

.. _sln_voice_asr_deploying_linux_macos_programming_guide:

******************************************
Deploying the Firmware with Linux or macOS
******************************************

This document explains how to deploy the software using ``CMake`` and ``Make``. 

Building the Host Server
========================

This application requires a host application to serve files to the device. The served file must be named ``test.wav``.  This filename is defined in ``src/app_conf.h``.

Run the following commands in the root folder to build the host application using your native Toolchain:

.. note::

  Permissions may be required to install the host applications.

.. code-block:: console

  cmake -B build_host
  cd build_host
  make xscope_host_endpoint
  make install

The host application, ``xscope_host_endpoint``, will be installed at ``/opt/xmos/bin``, and may be moved if desired.  You may wish to add this directory to your ``PATH`` variable.

Before running the host application, you may need to add the location of ``xscope_endpoint.so`` to your ``LD_LIBRARY_PATH`` environment variable.  This environment variable will be set if you run the host application in the XTC Tools command-line environment.  For more information see `Configuring the command-line environment <https://www.xmos.ai/documentation/XM-014363-PC-LATEST/html/tools-guide/install-configure/getting-started.html>`__.

Building the Firmware
=====================

Run the following commands in the root folder to build the firmware:

.. code-block:: console

    cmake -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
    cd build
    make example_asr

Flashing the Model
==================

Run the following commands in the root folder to flash the model:

.. code-block:: console

    xflash --quad-spi-clock 50MHz --factory example_asr.xe --boot-partition-size 0x100000 --target-file examples/speech_recognition/XCORE-AI-EXPLORER.xn --data examples/speech_recognition/asr/port/simple/simple_asr_model.dat

Running the Firmware
====================

From the root folder run:

.. code-block:: console

    make run_example_asr
