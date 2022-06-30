##############
Check Pipeline
##############

This script runs checks for the following pipelines:

- STLP-UA ADEC
- STLP-UA ADEC AtlArch

*********************
Install Prerequisites
*********************

To install the Sensory SDK, run the following command:

.. code-block:: console

    git clone git@github.com:xmos/sensory_sdk.git

Make the Sensory program executable:

.. tab:: Linux

    .. code-block:: console

        chmod a+x <path-to-sensory-sdk>/spot_eval_exe/spot-eval_x86_64-pc-linux-gnu

.. tab:: Mac

    .. code-block:: console

        chmod a+x <path-to-sensory-sdk>/spot_eval_exe/spot-eval_x86_64-apple-darwin

******************
Building the Tests
******************

To build the test application firmware, run the following command from the top of the repository: 

.. code-block:: console

    bash tools/ci/build_tests.sh

The `build_test.sh` script will copy the test applications to the `dist` folder.  

******************
Running the Tests
******************

First, run application firmware with the following command from the top of the repository:

.. code-block:: console

    xrun --xscope dist/<configuration>.xe

Then, in a separate terminal, run the test with the following command from the top of the repository:

.. code-block:: console

    bash test/pipeline/check_pipeline.sh <path-to-input-dir> <path-to-output-dir> <path-to-sensory-sdk>