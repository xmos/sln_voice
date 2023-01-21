############################
Check Sample Rate Conversion
############################

This test is a verification of the FFVA sample rate conversion feature.  

**************************
Building and Running Tests
**************************

.. note::

    The Python environment is required to run this test.  See the Requirements section of test/README.rst

To build the test application firmware and filesystem files, run the following command from the top of the repository: 

.. code-block:: console

    bash tools/ci/build_tests.sh

The `build_test.sh` script will copy the test applications and filesystem files to the `dist` folder.

Run the test with the following command from the top of the repository:

.. code-block:: console

    bash test/sample_rate_conversion/check_sample_rate_conversion.sh <firmware> <path-to-output-dir>

All paths must be absolute.  Relative paths may cause errors.  