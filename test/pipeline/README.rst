##############
Check Pipeline
##############

This script runs checks for the following pipelines:

- STLP-UA ADEC
- STLP-UA ADEC AtlArch

*********************
Install Prerequisites
*********************

Install `Docker <https://www.docker.com/>`_.

Pull the docker container:

.. code-block:: console

    docker pull debian:buster-slim

To install the Amazon WWE, run the following command:

.. code-block:: console

    git clone git@github.com:xmos/amazon_wwe.git

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

    bash test/pipeline/check_pipeline.sh <path-to-input-dir> <path-to-output-dir> <path-to-amazon-wwe>