##############
Check Pipeline
##############

*********************
Install Prerequisites
*********************

MacOS users need to install `Docker <https://www.docker.com/>`_.  Once Docker is installed, pull the required docker container:

.. code-block:: console

    docker pull debian:buster-slim

All users need to clone the Amazon WWE repository, run the following command:

.. code-block:: console

    git clone git@github.com:xmos/amazon_wwe.git

Note, the Amazon WWE requires a CPU with AVX2 support.  You can check for this with the following commands:

On Linux:

.. code-block:: console

    grep avx2 /proc/cpuinfo

On MacOS:

.. code-block:: console

    sysctl -a | grep machdep.cpu.leaf7_features | grep AVX2

If these commands return nothing then your computer lacks AVX2 support.  

******************
Building the Tests
******************

Begin by ensuring the filesystem is flashed.  To do this run the following commands from the top of the repository:

.. code-block:: console
    
    cmake -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
    cd build
    make flash_fs_example_stlp_ua_adec -j

To build the test application firmware, run the following command from the top of the repository: 

.. code-block:: console

    bash tools/ci/build_tests.sh

The `build_test.sh` script will copy the test applications to the `dist` folder.  

**********************
Running Pipeline Tests
**********************

First, run application firmware with the following command from the top of the repository:

.. code-block:: console

    xrun --xscope dist/<configuration>.xe

Then, in a separate terminal, run the test with the following command from the top of the repository:

.. code-block:: console

    bash test/pipeline/check_pipeline.sh <path-to-input-dir> <path-to-input-list> <path-to-output-dir> <path-to-amazon-wwe>

The <path-to-input-list> file is a text file listing wav files that must exist in <path-to-input-dir>.  The format of the file is:

    filename    AEC    min_instances    max_instances 

The filename must not include the ".wav" extension, values for AEC must be "Y" or "N".  
