############################
Check Sample Rate Conversion
############################

This test is a verification of the STLP sample rate conversion feature.  

******************
Building the Tests
******************

Begin by ensuring the filesystem is flashed.  To do this run the following commands from the top of the repository:

.. code-block:: console
    
    cmake -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
    cd build
    make flash_fs_example_ffd -j

To build the test application firmware, run the following command from the top of the repository: 

.. code-block:: console

    bash tools/ci/build_tests.sh

The `build_test.sh` script will copy the test applications to the `dist` folder.  

*************
Running Tests
*************

.. note::

    The Python environment is required to run this test.  See the Requirements section of test/README.rst

To build the test application firmware and filesystem files, run the following command from the top of the repository: 

.. code-block:: console

    bash tools/ci/build_tests.sh

The `build_test.sh` script will copy the test applications and filesystem files to the `dist` folder.

To flash the filesystem run:

.. code-block:: console
    
    xflash --quad-spi-clock 50MHz --factory dist/example_stlp_sample_rate_conv_test.xe --boot-partition-size 0x100000 --data dist/example_stlp_ua_adec_fat.fs


Run the test with the following command from the top of the repository:

.. code-block:: console

    bash test/sample_rate_conversion/check_sample_rate_conversion.sh <firmware> <path-to-output-dir>

All paths must be absolute.  Relative paths may cause errors.  