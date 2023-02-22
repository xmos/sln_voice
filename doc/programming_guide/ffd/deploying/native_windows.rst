.. include:: ../../../substitutions.rst

.. _sln_voice_ffd_deploying_native_windows:

******************************************
Deploying the Firmware with Native Windows
******************************************

This document explains how to deploy the software using `CMake` and `NMake`. If you are not using native Windows MSVC build tools and instead using a Linux emulation tool such as WSL, refer to :doc:`Deploying the Firmware with Linux or macOS <linux_macos>`.

Building the Firmware
=====================

Run the following commands in the root folder to build the firmware:

.. code-block:: console

    cmake -G "NMake Makefiles" -B build -D CMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
    cd build
    nmake example_ffd

Running the Firmware
====================

Before running the firmware, the filesystem and model must be flashed to the
data partition.

Within the root of the build folder, run:

.. code-block:: console

    nmake flash_app_example_ffd

After this command completes, the application will be running.

After flashing the data partition, the application can be run without
reflashing. If changes are made to the data partition components, the
application must be reflashed.

From the build folder run:

.. code-block:: console

    nmake run_example_ffd

Debugging the Firmware
======================

To debug with xgdb, from the build folder run:

.. code-block:: console

    nmake debug_example_ffd
