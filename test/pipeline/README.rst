##############
Check Pipeline
##############

This test is a verification of the FFD or FFVA audio pipelines.  

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

To build the test application firmware and filesystem files, run the following command from the top of the repository: 

.. code-block:: console

    bash tools/ci/build_tests.sh

The `build_test.sh` script will copy the test applications and filesystem files to the `dist` folder.  

To flash the filesystem:

If testing the FFVA pipeline run:

.. code-block:: console
    
    xflash --quad-spi-clock 50MHz --factory dist/example_ffva_ua_adec_test.xe --boot-partition-size 0x100000 --data dist/example_ffva_ua_adec_fat.fs

If testing the FFD pipeline run:

.. code-block:: console
    
    xflash --quad-spi-clock 50MHz --factory dist/example_ffd_usb_audio_test.xe --boot-partition-size 0x100000 --data dist/example_ffd_fat.fs

*************
Running Tests
*************

Run the test with the following command from the top of the repository:

.. code-block:: console

    bash test/pipeline/check_pipeline.sh <firmware> <path-to-input-dir> <path-to-input-list> <path-to-output-dir> <path-to-amazon-wwe>

All paths must be absolute.  Relative paths may cause errors.  

The <path-to-input-list> file is a text file listing wav files that must exist in <path-to-input-dir>.  The format of the file is:

    filename    AEC    min_instances    max_instances 

The filename must not include the ".wav" extension, values for AEC must be "Y" or "N".  
