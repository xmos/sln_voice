
##############
Check Commands
##############

This test is a verification of the FFD ASR command recognition.  

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

Run the test with the following command from the top of the repository:

.. code-block:: console

    bash test/commands/check_commands.sh <firmware> <path-to-input-dir> <path-to-input-list> <path-to-output-dir>

All paths must be absolute.  Relative paths may cause errors.  

The <path-to-input-list> file is a text file listing wav files that must exist in <path-to-input-dir>.  The format of the file is:

    filename    min_instances    max_instances 

Note, max_instances should be 50 or less because the firmware will not recognize more than 50 commands.
