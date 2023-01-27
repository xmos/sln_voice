
##############
Check Commands
##############

This test is a verification of the FFD ASR command recognition.  

******************
Building the Tests
******************

To build the test application firmware and filesystem files, run the following command from the top of the repository: 

.. code-block:: console

    bash tools/ci/build_tests.sh

The `build_test.sh` script will copy the test applications and filesystem files to the `dist` folder.  

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

The commands detections log can be verified via a pytest:

.. code-block:: console

    pytest test/pipeline/test_pipeline.py --log <path-to-output-dir>/results.csv