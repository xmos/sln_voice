############################
XCORE-VOICE Functional Tests
############################

************
Requirements
************

The following software is required for running many functional tests.  Install them using your operating systems package management system.

* Linux or MacOS. Windows is not currently supported.
* `Python 3.8 <https://www.python.org/>`__

Python Virtual Environment
==========================

While not required, we recommend you setup a Python virtual environment, like `Anaconda <https://www.anaconda.com/products/individual/>`_. If necessary, download and follow Anaconda's installation instructions.

Run the following command to create a Conda environment:

.. code-block:: console

    conda create --prefix sln_voice_venv python=3.8

Run the following command to activate the Conda environment:

.. code-block:: console

    conda activate <path-to>/sln_voice_venv

Run the following command to deactivate the Conda environment:

.. code-block:: console

    conda deactivate

Install Python Packages
=======================

Install development packages with the Python virtual environment activated:

.. code-block:: console

    pip install -r test/requirements.txt

*************
Running Tests
*************

Functional tests exists for the following:

- Audio processing pipelines
- Speech recognition command dictionaries
- Sample rate conversion

To run tests, see the README files located in the directories containing the tests.
