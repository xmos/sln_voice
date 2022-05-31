============================
Keyword Spotter Example Design
============================

This is the XMOS Keyword spotter example design.

******************
Supported Hardware
******************

This example is supported on the XK_VOICE_L71 board.

*********************
Building the Firmware
*********************

Run the following commands in the root folder to build the firmware:

.. tab:: Linux and Mac

    .. code-block:: console

        cmake -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
        cd build
        make example_keyword_spotter

.. tab:: Windows

    .. code-block:: console

        cmake -G "NMake Makefiles" -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
        cd build
        nmake example_keyword_spotter


********************
Running the Firmware
********************

From the build folder run:

.. tab:: Linux and Mac

    .. code-block:: console

        make run_example_keyword_spotter

.. tab:: Windows

    .. code-block:: console

        nmake run_example_keyword_spotter


********************************
Debugging the firmware with xgdb
********************************

From the build folder run:

.. tab:: Linux and Mac

    .. code-block:: console

        make debug_example_keyword_spotter

.. tab:: Windows

    .. code-block:: console

        nmake debug_example_keyword_spotter
