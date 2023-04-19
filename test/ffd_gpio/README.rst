########
FFD GPIO
########

*******
Purpose
*******

Description
===========

The FFD GPIO unit test is designed to verify the behavior of void proc_keyword_res(void *args).

Method
======


Inputs
======


Outputs
=======

*************
Running Tests
*************

This test runs on `xsim`.  Run the test with the following command from the top of the repository:

.. code-block:: console

    bash test/ffd_gpio/run_tests.sh

The results may be parsed and verified via a pytest:

.. code-block:: console

python tools/ci/python/parse_test_output.py testing/test.rpt -outfile="<output_dir>/output_file" --print_test_results --verbose