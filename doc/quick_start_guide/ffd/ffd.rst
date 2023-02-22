.. include:: ../../substitutions.rst
.. include:: <isonum.txt>

.. _sln_voice_ffd_quick_start:

#############################
Far-field Voice Local Command
#############################

********
Overview
********

These are the XCORE-VOICE far-field local control example designs demonstrating:

- 2-microphone far-field voice control with |I2C| or UART interface 
- Audio pipeline including interference cancelling and noise suppression 
- 25-phrase English language speech recognition

***************
Example designs
***************

Low-power Wake-up Demonstration
===============================

This is the low-power far-field voice local command (FFD) example design with Wanson speech recognition and local dictionary.

While inactive, low-power mode uses a fraction of energy otherwise required by normal operations while awaiting and processing speech.

When a wake-up phrase is followed by an command phrase, the application will output an audio response and a discrete message over |I2C| and UART.

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

Connect the XTAG4 via USB to the host computer running the XTC tools, and power on the board directly via USB.

On the host computer, open a ``XTC Tools Command Prompt``.

.. code-block:: console

    xflash --quad-spi-clock 50MHz --factory example_ffd.xe --boot-partition-size 0x100000 --data example_ffd_data_partition.bin

Being returned to the prompt means flashing has completed, and the XTAG4 may be disconnected.

Speech Recognition
^^^^^^^^^^^^^^^^^^

Speak one of the wakewords followed by one of the commands from the lists below.

There are three LED states:

- Flashing Green    = Full Power, Waiting for Wake Word
- Solid Red & Green = Full Power, Waiting for Command
- Solid Red         = Low Power

The application rests in low-power mode (solid red) until the audio pipeline detects audio, thereby entering full-power mode (flashing green) to begin wake-up phrase recognition.
Upon recognizing 'Hello XMOS,' waiting begins for a command (solid red & green).
After a period of inactivity, low-power mode resumes.

**Wakewords**

- Hello XMOS

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

Test Wake-up and Low-power Functionality
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1. Once flashing is complete, the application is now running on the board.

2. Observe application state. While not detecting sufficient acoustic activity, the demo enters low-power mode. Observe the solid red LED.

3. Say, "Far field voice local control". The demo enters full-power mode, waiting for the wake-up phrase. Observe the flashing green LED.

4. Speak the wake-up phrase, "Hello XMOS". The demo plays a recognition tone and awaits a command for a time. Observe the solid red and green LEDs.

5. Say, "Switch on the lights". The demo recognizes this command, and replies an acknowledgement over speakers, |I2C| and UART.

6. The demo awaits more commands. Say, "Volume up". After another acknowledgement, the board will continue to wait for commands.

7. After a period of inactivity, a power-down tone plays and low-power mode resumes.
