############################
Check Sample Rate Conversion
############################

This test is a verification of the STLP sample rate conversion feature.

**************************
Building and Running Tests
**************************

.. note::

    The Python environment is required to run this test.  See the Requirements section of test/README.rst

The `check_sample_rate_conversion.sh` script will build and flash the firmware, and copy the test applications to the `dist` folder.
Build and run the test with the following command from the top of the repository:

.. code-block:: console

    bash test/sample_rate_conversion/check_sample_rate_conversion.sh <firmware> <path-to-output-dir>

All paths must be absolute.  Relative paths may cause errors.  