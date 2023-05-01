###############
Check Pipelines
###############

*******
Purpose
*******

Description
===========

This test is a verification of the FFD or FFVA audio pipelines.  It verifies that the acoustic pipeline performance meets or exceeds the performance of the audio pipeline testing results from the Voice Framework.

Method
======

Play test recordings to a test configuration built to read input microphone and reference audion from WAV files using xscope_fileio.  Process the inputs WAV files with a configured pipeline (AEC (optional) + IC/VNR + NS + AGC) and save the processed output channels.  Process with output wav with the `x86/amazon_ww_filesim` application from the Amazon WWE using the `models/common/WR_250k.en-US.alexa.bin`` model.  Count the number of recognized “alexa” utterances.  

Inputs
======

The input wav files are listed in the test vector file lists: 

- ffd_quick.txt
- ffva_quick.txt

The wav files are 2 minute subsets from the xvf3510_no_processing_xmos_test_suite test set.  Files can be copied from: `\\projects.xmos.local\projects\hydra_audio\xcore-voice_xvf3510_no_processing_xmos_test_suite_subset`

Outputs
=======

The minimum number of recognized “alexa”s is specified in the test vector file lists.

Intermediate and output `wav` files are saved in the output directory for manual inspection if necessary.

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

*************
Running Tests
*************

Run the test with the following command from the top of the repository:

.. code-block:: console

    bash test/pipeline/check_pipeline.sh <firmware> <path-to-input-dir> <path-to-input-list> <path-to-output-dir> <path-to-amazon-wwe>

All paths must be absolute.  Relative paths may cause errors.  

The <path-to-input-list> file is a text file listing wav files that must exist in <path-to-input-dir>.  The format of the file is:

.. code-block:: console

    filename    AEC    min_instances    max_instances 

The filename must not include the `.wav`` extension, values for AEC must be "Y" or "N".  

The pipeline detections log can be verified via a pytest:

.. code-block:: console

    pytest test/pipeline/test_pipeline.py --log <path-to-output-dir>/results.csv