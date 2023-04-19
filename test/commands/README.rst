
##############
Check Commands
##############

*******
Purpose
*******

Description
===========

This test is a verification of the FFD ASR command recognition.  

Method
======

Plays a test recording to a test configuration built to use two USB audio input channels as microphone inputs.  Then captures the `xrun` console output and parses it for recognition event log messages.  

Inputs
======

The input wav files are listed in the test vector file lists: 

- ffd.txt

The wav files are can be copied from: `\\projects.xmos.local\projects\hydra_audio\xcore-voice_xvf3510_no_processing_xmos_test_suite_subset`

Outputs
=======

The minimum number of recognition events is specified in the test vector file lists.

Intermediate and output `wav` files are saved in the output directory for manual inspection if necessary.

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