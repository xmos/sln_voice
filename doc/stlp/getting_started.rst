.. include:: ../substitutions.rst

.. _sln_voice_getting_started:

####################################################################
Getting Started with the XCORE-VOICE Reference Design Evaluation Kit
####################################################################

.. toctree::
   :maxdepth: 1
   :hidden:
   
==============
Demonstrations
==============

STLP-UA - direct connection over USB to the host allowing signal analysis and evaluation

STLP-INT - integration into RPi system, using I2S, running an AVS client

------------------
Supported Hardware
------------------

These demos are supported on the `XK-VOICE-L71 <https://www.digikey.co.uk/en/products/detail/xmos/XK-VOICE-L71/15761172>`_ board.

---------------------------
Getting the Latest Firmware
---------------------------

To flash the firmware onto the XK-VOICE-L71 or swap between -UA and -INT configurations follow these steps:

On GitHub
---------

Get the latest version from `sln_voice <https://github.com/xmos/sln_voice>`_

Follow the *readme* instructions on how to clone this repo.

Checkout the tagged versions for the latest stable release.

Download the XTC Tools from xmos.ai/tools on your chosen host.

Connect either end of the ribbon cable to the XTAG4, and the other end to the XK-VOICE-L71 board as shown (Image shows piggybacked connection to RPi. Standalone operation is also supported):

.. image:: images/getting_started/XMOS_XK_VOICE_L71_Rev2_5N2A8560_2048px.jpg
  :width: 800
  :alt: XK-VOICE-L71 on RPi with ribbon cable

Building the Firmware
---------------------

Connect the XTAG4 via USB to the host computer running the XTC tools, and power on the board (either via RPi or directly via USB).

On the host computer, open a ‘XTC Tools 15.1.0 Command Prompt’.

Navigate to the root directory of the sln_voice repository.

Run the following commands to build the firmware. Both -UA and -INT configurations may be built for flashing, but only one may be flashed onto the board at any given time.

.. tab:: Linux and Mac

    .. code-block:: console

        cmake -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
        cd build

        make application_stlp_int_adec
        make application_stlp_ua_adec

.. tab:: Windows

    .. code-block:: console

        cmake -G "NMake Makefiles" -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
        cd build

        nmake application_stlp_int_adec
        nmake application_stlp_ua_adec

From the build folder, create the filesystem and flash the device with the appropriate command to the desired configuration:

.. tab:: Linux and Mac

    .. code-block:: console

        make flash_fs_application_stlp_int_adec
        make flash_fs_application_stlp_ua_adec

.. tab:: Windows

    .. code-block:: console

        nmake flash_fs_application_stlp_int_adec
        nmake flash_fs_application_stlp_ua_adec

===========================
Standalone UA Demonstration
===========================

.. tab:: Requirements

    XK-VOICE-L71 flashed with STLP-UA firmware
    
    Powered speaker(s) with 3.5mm jack connection
    
    Host system running Windows, macOS, Linux or Android
    
    USB A to Micro cable for connection to the host

----------------------
Configure the Hardware
----------------------

Connect the host system to the micro-USB socket, and the speakers to the jack plug as shown:

.. image:: images/getting_started/XMOS_XK_VOICE_L71_Rev2_5N2A8765_2048px.jpg
  :width: 800
  :alt: XK-VOICE-L71 connected to powered speakers and host device

Either mono or stereo speakers may be used.

---------------------
Record Captured Voice
---------------------

1. Open a music player on host PC, and play a stereo file.

2. Check music is playing through powered speakers.

3. Adjust volume using music player or speakers.

4. Open Audacity and configure to communicate with kit. Input Device: XCORE-VOICE Voice Processor and Output Device: XCORE-VOICE Voice Processor

5. Set recording channels to 2 (Stereo) in Device

.. image:: images/getting_started/channels_dropdown.png
  :width: 800
  :alt: audacity channels dropdown
  
6. Set Project Rate to 48000Hz in Selection Toolbar.

.. image:: images/getting_started/audacity-rate.png
  :width: 230
  :alt: audacity bitrate setting
  
7. Click Record (press 'r') to start capturing audio streamed from the XCORE-VOICE device.

8. Talk over music; move around the room while talking.

9. Stop music player.

10. Click Stop (press space) to stop recording. Audacity records single audio channel streamed from the XCORE-VOICE kit including extracted voice signal.

11. Click dropdown menu next to Audio Track, and select Split Stereo To Mono.

.. image:: images/getting_started/split-track-to-mono.jpg
  :width: 400
  :alt: audacity split action dropdown
  
12. Click Solo on left channel of split processed audio. Increase Gain slider if necessary.

.. image:: images/getting_started/solo-gain.png
  :width: 400
  :alt: audacity solo and gain options
  
13. Click Play (press space) to playback processed audio.

Only your voice is audible. Playback music is removed by acoustic echo cancellation; voice is isolated by interference canceller; background noise is removed by noise suppression algorithms.

===================================
Integrated Amazon AVS Demonstration
===================================

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

Prepare the Raspberry Pi System image on the SD card as described on `github <https://github.com/xmos/vocalfusion-avs-setup>`_

---------------------
Connect the System
---------------------

Connect the speakers (into the XV-VOICE-71), HDMI monitor cable, and mouse as shown:

.. image:: images/getting_started/XMOS_XK_VOICE_L71_Rev2_5N2A8758_2048px.jpg
  :width: 800
  :alt: XK-VOICE-L71 in INT hardware configuration
  
---------------------
Install and Configure
---------------------

Note: *The STLP-INT firmware is compatible with XVF3610-INT software, therefore instructions for installing the XVF3610-INT pi software can be followed for this AVS demo. The "Firmware Upgrade" section may be dismissed, as your STLP-INT firmware is already updated per the above section of this guide.*

Install the Amazon Alexa SDK and configure the Raspberry Pi Audio by following the instructions here:

`AVS Setup Instructions <https://github.com/xmos/vocalfusion-avs-setup>`_

--------
Run Demo
--------

Once the installation is complete, run the demo by typing *avsrun* in a terminal. The demo will now operate as an Alexa virtual assistant.
