====================================
XCORE-VOICE Host Control Application
====================================

This is the XCORE-VOICE host control application.


***** 
Setup
***** 

This application requires the xcore_sdk and Amazon Wakeword.

LibUSB 1.0 is required for Linux. Install these packages:

- libusb-dev
- libusb-1.0-0-dev  

************************
Building the Application
************************

To build this application, run the following commands in the host/control/ directory:

.. tab:: Linux and MacOS

    .. code-block:: console
    
        $ cmake -B build
        $ cd build
        $ make -j
        
.. tab:: Windows x86 native tools CMD prompt

    .. code-block:: console
    
        $ cmake -G "NMake Makefiles" -B build
        $ cd build
        $ nmake


*******************
Verify Installation
*******************

From the root folder of the application, run:

.. tab:: Linux and MacOS

    .. code-block:: console

        $ avona_control --help
        
.. tab:: Windows x86 native tools CMD prompt

    .. code-block:: console
    
        $ avona_control.exe --help
