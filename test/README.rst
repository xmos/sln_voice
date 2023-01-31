#################
XCORE-VOICE Tests
#################

************
Requirements
************

The following software is required for running tests.

* Linux, MacOS, or Windows Subsystem for Linux
* `Python 3.8 <https://www.python.org/>`__

Python Virtual Environment
==========================

While not required, we recommend you setup a Python virtual environment, like `https://realpython.com/python-virtual-environments-a-primer/`

Install Python Packages
=======================

Install development packages with the Python virtual environment activated:

.. code-block:: console

    pip install -r test/requirements.txt

*************
Running Tests
*************

Tests exists for the following:

- Audio processing pipelines
- Speech recognition command dictionaries
- Sample rate conversion
- DFU
- GPIO

To run tests, see the README files located in the directories containing each test group.
