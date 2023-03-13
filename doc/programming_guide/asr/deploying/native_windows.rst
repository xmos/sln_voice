.. include:: ../../../substitutions.rst

******************************************
Deploying the Firmware with Native Windows
******************************************

This document explains how to deploy the software using `CMake` and `NMake`. If you are not using native Windows MSVC build tools and instead using a Linux emulation tool, refer to :ref:`sln_voice_asr_deploying_linux_macos_programming_guide`.

Building the Host Server
========================

This application requires a host application to serve files to the device. The served file must be named ``test.wav``.  This filename is defined in ``src/app_conf.h``.

Run the following commands in the root folder to build the host application using your native Toolchain:

.. note::

  Permissions may be required to install the host applications.

Before building the host application, you will need to add the path to the XTC Tools to your environment.

.. code-block:: console

  set "XMOS_TOOL_PATH=<path-to-xtc-tools>"

Then build the host application:

.. code-block:: console

  cmake -G "NMake Makefiles" -B build_host
  cd build_host
  nmake xscope_host_endpoint
  nmake install

The host application, ``xscope_host_endpoint.exe``, will install at ``<USERPROFILE>\.xmos\bin``, and may be moved if desired.  You may wish to add this directory to your ``PATH`` variable.

Before running the host application, you may need to add the location of ``xscope_endpoint.dll`` to your ``PATH``. This environment variable will be set if you run the host application in the XTC Tools command-line environment.  For more information see `Configuring the command-line environment <https://www.xmos.ai/documentation/XM-014363-PC-LATEST/html/tools-guide/install-configure/getting-started.html>`__.

Building the Firmware
=====================

Run the following commands in the root folder to build the firmware:

.. code-block:: console

    cmake -G "NMake Makefiles" -B build -D CMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
    cd build
    nmake example_asr

.. _sln_voice_asr_programming_guide_flash_model:

Flashing the Model
==================

Run the following commands in the build folder to flash the model:

.. code-block:: console

    xflash --force --quad-spi-clock 50MHz --factory example_asr.xe --boot-partition-size 0x100000 --target-file ../examples/speech_recognition/XCORE-AI-EXPLORER.xn --data ../examples/speech_recognition/asr/port/example/asr_example_model.dat

Running the Firmware
====================

From the build folder run:

.. code-block:: console

    nmake run_example_asr

In a second console, run the following command in the ``examples/speech_recognition`` folder to run the host server:

.. code-block:: console
    
    xscope_host_endpoint.exe 12345
