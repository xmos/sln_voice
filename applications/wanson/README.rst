============================
Wanson App
============================

This is the XMOS FFD reference design with Wanson Keyword spotter

******************
Supported Hardware
******************

This example is supported on the XK_VOICE_L71 board.

*********************
Building the Firmware
*********************

Run the following commands in the xcore_sdk root folder to build the firmware:

.. tab:: Linux and Mac

    .. code-block:: console

        $ cmake -B build -DCMAKE_TOOLCHAIN_FILE=tools/xmos_cmake_toolchain/xs3a.cmake
        $ cd build
        $ make application_wanson

.. tab:: Windows

    .. code-block:: console

        $ cmake -G "NMake Makefiles" -B build -DCMAKE_TOOLCHAIN_FILE=tools/xmos_cmake_toolchain/xs3a.cmake
        $ cd build
        $ nmake application_wanson


********************
Running the Firmware
********************

Before the firmware is run, the swmem must be loaded.  This application currently uses a nonstandard swmem access, so the steps to setup the flash are dictated below.

Inside of the build folder root, after building the firmware:

.. tab:: Linux and Mac

    .. code-block:: console

        $ xobjdump --strip application_wanson.xe
        $ xobjdump --split application_wanson.xb
        $ xflash --boot-partition-size 0x100000 --data image_n0c0.swmem --factory application_wanson.xe --target-file platform.xn


.. tab:: Windows

    .. code-block:: console

        $ xobjdump --strip application_wanson.xe
        $ xobjdump --split application_wanson.xb
        $ xflash --boot-partition-size 0x100000 --data image_n0c0.swmem --factory application_wanson.xe --target-file platform.xn


From the xcore_sdk build folder run:

.. tab:: Linux and Mac

    .. code-block:: console

        $ make run_application_wanson

.. tab:: Windows

    .. code-block:: console

        $ nmake run_application_wanson


********************************
Debugging the firmware with xgdb
********************************

From the xcore_sdk build folder run:

.. tab:: Linux and Mac

    .. code-block:: console

        $ make debug_application_wanson

.. tab:: Windows

    .. code-block:: console

        $ nmake debug_application_wanson
