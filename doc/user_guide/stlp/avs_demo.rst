.. include:: ../../substitutions.rst

#######################################
Alexa Voice Service (AVS) Demonstration
#######################################

.. toctree::
   :maxdepth: 1
   :hidden:
   
========
Overview
========

Integration into RPi system, using I2S, running an Alexa Voice Service (AVS) client.

--------------
Hardware Setup
--------------

Connect either end of the ribbon cable to the XTAG4, and the other end to the XK-VOICE-L71 board as shown (Image shows piggybacked connection to RPi. Standalone operation is also supported):

.. image:: images/getting_started/XMOS_XK_VOICE_L71_Rev2_5N2A8560_2048px.jpg
  :width: 800
  :alt: XK-VOICE-L71 on RPi with ribbon cable

---------------------
Building the Firmware
---------------------

Connect the XTAG4 via USB to the host computer running the XTC tools, and power on the board (either via RPi or directly via USB).

On the host computer, open a `XTC Tools Command Prompt`.

Navigate to the root directory of the sln_voice repository.

Run the following commands to build the firmware. Both -UA and -INT configurations may be built for flashing, but only one may be flashed onto the board at any given time.

.. tab:: Linux and Mac

    .. code-block:: console

        cmake -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
        cd build

        make example_stlp_int_adec

.. tab:: Windows

    .. code-block:: console

        cmake -G "NMake Makefiles" -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
        cd build

        nmake example_stlp_int_adec

From the build folder, create the filesystem and flash the device with the appropriate command to the desired configuration:

.. tab:: Linux and Mac

    .. code-block:: console

        make flash_fs_example_stlp_int_adec

.. tab:: Windows

    .. code-block:: console

        nmake flash_fs_example_stlp_int_adec

=========================
Running the Demonstration
=========================

.. tab:: Requirements

    XK-VOICE-L71 flashed with STLP-INT firmware
    
    Powered speaker(s) with 3.5mm jack connection
    
    Raspberry Pi model 3 or 4 with power unit
    
    HDMI monitor, USB keyboard and mouse
    
    SD card (minimum 16GB size)
    
    Amazon Developer Account

`Detailed Instructions <https://github.com/xmos/vocalfusion-avs-setup>`_

---------------------
Assemble the Hardware
---------------------

Connect the XV-VOICE-L71 to the Raspberry Pi ensuring that the connector fully lines up, as shown below.

.. image:: images/getting_started/XMOS_XK_VOICE_L71_Rev2_5N2A8559_2048px.jpg
  :width: 800
  :alt: XK-VOICE-L71 piggybacked on RPi
  
-------------------------------
Prepare the Raspberry Pi System
-------------------------------

.. note:: The STLP-INT firmware is compatible with XVF3610-INT software, therefore instructions for installing the XVF3610-INT pi software can be followed for this AVS demo. The "Firmware Upgrade" section may be dismissed, as your STLP-INT firmware is already updated per the above section of this guide.

Prepare the Raspberry Pi System image on the SD card by following the instructions for XVF3610-INT as described on `github <https://github.com/xmos/vocalfusion-avs-setup>`_

------------------
Connect the System
------------------

Connect the speakers (into the XV-VOICE-71), HDMI monitor cable, and mouse as shown:

.. image:: images/getting_started/XMOS_XK_VOICE_L71_Rev2_5N2A8758_2048px.jpg
  :width: 800
  :alt: XK-VOICE-L71 in INT hardware configuration
  
---------------------
Install and Configure
---------------------

Install the Amazon Alexa SDK and configure the Raspberry Pi Audio by following the instructions here:

`AVS Setup Instructions <https://github.com/xmos/vocalfusion-avs-setup>`_

--------
Run Demo
--------

Once the installation is complete, run the demo by typing `avsrun` in a terminal. The demo will now operate as an Alexa virtual assistant.
