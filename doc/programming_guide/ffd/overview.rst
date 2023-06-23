.. include:: ../../substitutions.rst

.. _sln_voice_ffd_overview:

********
Overview
********

This is the far-field voice local command (FFD) example design with Sensory speech recognition using Sensory's TrulyHandsfree™ (THF) speech recognition and local dictionary.

When a wakeword phrase is detected followed by an command phrase, the application will output an audio response and a discrete message over |I2C| and UART.

Sensory's THF software ships with an expiring development license. It will suspend recognition after 11.4 hours or 107 recognition events; after which, a device reset is required to resume normal operation. To perform a reset, either power cycle the device or press the SW2 button.

More information on the Sensory speech recognition library can be found here: :ref:`sln_voice_ffd_speech_recognition`
