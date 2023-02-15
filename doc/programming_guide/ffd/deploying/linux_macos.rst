.. include:: ../../../substitutions.rst

******************************************
Deploying the Firmware with Linux or macOS
******************************************

This document explains how to deploy the software using `CMake` and `Make`. 

Building the Firmware
=====================

Run the following commands in the root folder to build the firmware:

.. code-block:: console

    cmake -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
    cd build
    make example_ffd

Running the Firmware
====================

Before the firmware is run, the filesystem must be loaded.

Inside of the build folder root, after building the firmware:

.. code-block:: console

    make flash_app_example_ffd

Once flashed, the application will run.

After the filesystem has been flashed once, the application can be run without flashing.  If changes are made to the filesystem image, the application must be reflashed.

From the build folder run:

.. code-block:: console

    make run_example_ffd

Debugging the Firmware
======================

To debug with xgdb, from the build folder run:


.. code-block:: console

    make debug_example_ffd
    