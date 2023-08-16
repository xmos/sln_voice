.. include:: ../../../substitutions.rst

.. _sln_voice_ffd_deploying_native_windows:

******************************************
Deploying the Firmware with Native Windows
******************************************

This document explains how to deploy the software using ``CMake`` and ``Ninja``. If you are not using native Windows build tools and instead using a Linux emulation tool such as WSL, refer to :doc:`Deploying the Firmware with Linux or macOS <linux_macos>`.

It is highly recommended to use ``Ninja`` as the make system under cmake. Not only is it a lot faster
than MSVC ``nmake``, it also works around an issue where certain path names may cause an issue with the XMOS compiler under windows.

To install Ninja, follow these steps:

- Download ``ninja.exe`` from https://github.com/ninja-build/ninja/releases. This firmware has been tested with Ninja version v1.11.1
- Ensure Ninja is on the command line path. You can add to the path permanently by following these steps https://www.computerhope.com/issues/ch000549.htm. Alternatively you may set the path in the current command line session using something like ``set PATH=%PATH%;C:\Users\xmos\utils\ninja``


.. note::

    In the commands below ``<speech_engine>`` can be either ``sensory`` or ``cyberon``, depending on the choice of the speech recognition engine and model.

Building the Host Applications
==============================

This application requires a host application to create the flash data partition. Run the following commands in the root folder to build the host application using your native Toolchain:

.. note::

  Permissions may be required to install the host applications.

Before building the host application, you will need to add the path to the XTC Tools to your environment.

.. code-block:: console

  set "XMOS_TOOL_PATH=<path-to-xtc-tools>"

Then build the host application:

.. code-block:: console

  cmake -G Ninja -B build_host
  cd build_host
  ninja install

The host applications will be installed at ``%USERPROFILE%\.xmos\bin``, and may be moved if desired.  You may wish to add this directory to your ``PATH`` variable.

Building the Firmware
=====================

Run the following commands in the root folder to build the firmware:

.. code-block:: console

    cmake -G Ninja -B build --toolchain  ..\xmos_cmake_toolchain\xs3a.cmake
    cd build
    ninja example_ffd_<speech_engine>

Running the Firmware
====================

Before running the firmware, the filesystem and model must be flashed to the data partition.

Within the root of the build folder, run:

.. code-block:: console

    ninja flash_app_example_ffd_<speech_engine>


After this command completes, the application will be running.

After flashing the data partition, the application can be run without reflashing. If changes are made to the data partition components, the application must be reflashed.

From the build folder run:

.. code-block:: console

    ninja run_example_ffd_<speech_engine>


Debugging the Firmware
======================

To debug with xgdb, from the build folder run:

.. code-block:: console

    ninja debug_example_ffd_<speech_engine>
:browse confirm wa

