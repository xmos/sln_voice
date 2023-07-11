#########
Check ASR
#########

*******
Purpose
*******

Description
===========

This test is a verification of the ASR libraries.  It verifies that the ASR recognition rate meets minimum requirements.

Method
======

Play test recordings to a test configuration built to use two USB audio input channels as microphone inputs.  Process the two channel inputs with the full pipeline (AEC (optional) + IC/VNR + NS + AGC) and record the processed ASR output channel.  Process with output wav with the `x86/amazon_ww_filesim` application from the Amazon WWE using the `models/common/WR_250k.en-US.alexa.bin`` model.  Count the number of recognized “alexa” utterances.  

Inputs
======

The input wav files are listed in the test vector file lists: 

- ffd.txt

The wav files are 2 minute subsets from the xvf3510_no_processing_xmos_test_suite test set.  Files can be copied from: `\\projects.xmos.local\projects\hydra_audio\xcore-voice_xvf3510_no_processing_xmos_test_suite_subset`

Outputs
=======

The minimum number of recognized events is specified in the test vector file lists.

*************
Running Tests
*************

Run the test with the following command from the top of the repository:

.. code-block:: console

    bash test/asr/check_asr.sh <asr-library> <path-to-input-dir> <path-to-input-list> <path-to-output-dir>

All paths must be absolute.  Relative paths may cause errors.  

The <path-to-input-list> file is a text file listing wav files that must exist in <path-to-input-dir>.  The format of the file is:

.. code-block:: console

    Filename    Max_Allowable_WER 

The filename must not include the `.wav`` extension.  

The ASR detections log can be verified via a pytest:

.. code-block:: console

    pytest test/asr/test_asr.py --log <path-to-output-dir>/results.csv