.. include:: ../../substitutions.rst

**********************
Deploying the Software
**********************

Building the Firmware
=====================

Run the following commands in the root folder to build the |I2S| firmware:

**Linux and Mac**

.. code-block:: console

    cmake -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
    cd build
    make example_ffva_int_adec

**Windows**

.. code-block:: console

    cmake -G "NMake Makefiles" -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
    cd build
    nmake example_ffva_int_adec


Run the following commands in the root folder to build the USB firmware:

**Linux and Mac**

.. code-block:: console

    cmake -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
    cd build
    make example_ffva_ua_adec

**Windows**

.. code-block:: console

    cmake -G "NMake Makefiles" -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
    cd build
    nmake example_ffva_ua_adec

Running the Firmware
====================

Before the firmware is run, the filesystem must be loaded.

Inside of the build folder root, after building the firmware, run one of:

**Linux and Mac**

.. code-block:: console

    make flash_fs_example_ffva_int_adec
    make flash_fs_example_ffva_ua_adec

**Windows**

.. code-block:: console

    nmake flash_fs_example_ffva_int_adec
    nmake flash_fs_example_ffva_ua_adec

Once flashed, the application will run.

After the filesystem has been flashed once, the application can be run without flashing.  If changes are made to the filesystem image, the application must be reflashed.

From the build folder run:

**Linux and Mac**

.. code-block:: console

    make run_example_ffva_int_adec
    make run_example_ffva_ua_adec

**Windows**

.. code-block:: console

    nmake run_example_ffva_int_adec
    nmake run_example_ffva_ua_adec

Upgrading the Firmware
======================

The UA variants of this application contain DFU over the USB DFU Class V1.1 transport method.

To create an upgrade image from the build folder run:

**Linux and Mac**

.. code-block:: console

    make create_upgrade_img_example_ffva_ua_adec
    make create_upgrade_img_example_ffva_ua_adec_altarch

**Windows**

.. code-block:: console

    nmake create_upgrade_img_example_ffva_ua_adec
    nmake create_upgrade_img_example_ffva_ua_adec_altarch

Once the application is running, a USB DFU v1.1 tool can be used to perform various actions.  This example will demonstrate with dfu-util commands.  Installation instructions for respective operating system can be found `here <https://dfu-util.sourceforge.net/>`__

To verify the device is running run:

.. code-block:: console

    dfu-util -l

This should result in an output containing:

.. code-block:: console

    Found DFU: [20b1:0020] ver=0001, devnum=100, cfg=1, intf=3, path="3-4.3", alt=2, name="DFU DATAPARTITION", serial="123456"
    Found DFU: [20b1:0020] ver=0001, devnum=100, cfg=1, intf=3, path="3-4.3", alt=1, name="DFU UPGRADE", serial="123456"
    Found DFU: [20b1:0020] ver=0001, devnum=100, cfg=1, intf=3, path="3-4.3", alt=0, name="DFU FACTORY", serial="123456"

The DFU interprets the flash as 3 separate partitions, the read only factory image, the read/write upgrade image, and the read/write data partition containing the filesystem.

The factory image can be read back by running:

.. code-block:: console

    dfu-util -e -d 0020 -a 0 -U readback_factory_img.bin

The factory image can not be written to.

From the build folder, the upgrade image can be written by running:

.. code-block:: console

    dfu-util -e -d 0020 -a 1 -D example_ffva_ua_adec_upgrade.bin
    dfu-util -e -d 0020 -a 1 -D example_ffva_ua_adec_altarch_upgrade.bin

The upgrade image can be read back by running:

.. code-block:: console

    dfu-util -e -d 0020 -a 1 -U readback_upgrade_img.bin

On system reboot, the upgrade image will always be loaded if valid.  If the upgrade image is invalid, the factory image will be loaded.  To revert back to the factory image, you can upload an file containing the word 0xFFFFFFFF.

The data partition image can be read back by running:

.. code-block:: console

    dfu-util -e -d 0020 -a 2 -U readback_data_partition_img.bin

The data partition image can be written by running:

.. code-block:: console

    dfu-util -e -d 0020 -a 2 -D readback_data_partition_img.bin

Note that the data partition will always be at the address specified in the initial flashing call.


Debugging the Firmware
======================

To debug with xgdb, from the build folder run:

**Linux and Mac**

.. code-block:: console

    make debug_example_int_adec
    make debug_example_ua_adec

**Windows**

.. code-block:: console

    nmake debug_example_int_adec
    nmake debug_example_ua_adec
