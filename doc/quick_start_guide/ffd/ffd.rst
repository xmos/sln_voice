.. include:: ../../substitutions.rst
.. include:: <isonum.txt>

.. _sln_voice_ffd_quick_start:

#############################
Far-field Voice Local Control
#############################

********
Overview
********

These are the XCORE-VOICE far-field local control example designs demonstrating:

- 2-microphone far-field voice control with |I2C| or UART interface 
- Audio pipeline including interference cancelling and noise supression 
- 25-phrase English language voice recognition

***************
Example designs
***************

Low-power Wake-up Demonstration
===============================

This is the low-power far-field voice local command (FFD) example design with Wanson speech recognition and local dictionary.

TODO: Add info on low-power capabilities.  

When a wakeup phrase is followed by an intent phrase the application will output an audio response and a discrete message over |I2C| and UART.

This software is an evaluation version only.  It includes a mechanism that limits the maximum number of recognitions to 50. You can reset the counter to 0 by restarting or rebooting the application.  The application can be rebooted by power cycling or pressing the SW2 button.

**Requirements**

- XK-VOICE-L71 board
- Powered speaker(s) with 3.5mm jack connection (OPTIONAL)

Hardware Setup
--------------

This example design requires an XTAG4 and XK-VOICE-L71 board.

.. image:: ../../shared/images/all_components.jpg
  :width: 800
  :alt: all components

Connect the xTAG to the debug header, as shown below.

.. image:: ../../shared/images/xtag_installation.jpg
  :width: 800
  :alt: xtag

Connect the micro USB XTAG4 and micro USB XK-VOICE-L71 to the programming host.

.. image:: ../../shared/images/host_setup.jpg
  :width: 800
  :alt: programming host setup

Speakers (OPTIONAL)
^^^^^^^^^^^^^^^^^^^

This example application features audio playback responses.  Speakers can be connected to the LINE OUT on the XK-VOICE-L71.

Running the Demonstration
-------------------------

Flashing the Firmware
^^^^^^^^^^^^^^^^^^^^^

Connect the XTAG4 via USB to the host computer running the XTC tools, and power on the board (either via RPi or directly via USB).

On the host computer, open a `XTC Tools Command Prompt`.

.. code-block:: console

    xflash --quad-spi-clock 50MHz --factory example_ffd.xe --boot-partition-size 0x100000 --data example_ffd_fat.fs

Speech Recognition
^^^^^^^^^^^^^^^^^^

Speak one of the wakewords followed by one of the commands from the lists below.

TODO: Explain LED status and how it relates to power management state.  

**Wakewords**

- Hello XMOS
- Hello Wanson

**Dictionary Commands**

- Switch on the TV
- Switch off the TV
- Channel up
- Channel down
- Volume up
- Volume down
- Switch on the lights
- Switch off the lights
- Brightness up
- Brightness down
- Switch on the fan
- Switch off the fan
- Speed up the fan
- Slow down the fan
- Set higher temperature
- Set lower temperature
