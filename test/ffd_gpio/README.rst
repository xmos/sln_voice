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


******************
Building the Tests
******************

To build the test application firmware and filesystem files, run the following command from the top of the repository: 

.. code-block:: console

    bash tools/ci/build_tests.sh

The `build_test.sh` script will copy the test applications and filesystem files to the `dist` folder.  

*************
Running Tests
*************

This test runs on `xsim`.  Run the test with the following command from the top of the repository:

.. code-block:: console

    bash test/ffd_gpio/run_tests.sh
    