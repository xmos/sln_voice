
.. _sln_voice_ffd_overview:

********
Overview
********

This is the far-field voice local command (FFD) example design. Three examples are provided: all examples include speech recognition and a local dictionary. One example uses the Sensory TrulyHandsfree™ (THF) libraries, and the other ones use the Cyberon DSPotter™ libraries. The two examples with the Cyberon DSPotter™ libraries differ in the audio source fed into the intent engine. One example uses the audio source from the microphone array, and the other uses the audio source from the |I2S| interface.

The examples using the microphone array as the audio source include an audio pipeline with the following stages:

    #. Interference Canceler (IC) + Voice To Noise Ratio Estimator (VNR)
    #. Noise Suppressor (NS)
    #. Adaptive Gain Control (AGC)

The FFD examples provide several options to inform the host of a possible intent detected by the intent engine. The device can notify the host by:

  - sending the intent ID over a UART interface upon detecting the intent
  - sending the intent ID over an |I2C| master interface upon detecting the intent
  - allowing the host to poll the last detected intent ID over the |I2C| slave interface
  - listening to an audio message over an |I2S| interface

When a wakeword phrase is detected followed by a command phrase, the application will output an audio response and a discrete message over |I2C| and UART.

Sensory's THF and Cyberon's DSpotter™ libraries ship with an expiring development license. The Sensory one will suspend recognition after 11.4 hours or 107 recognition events, and the Cyberon one will suspend recognition after 100 recognition events. After the maximum number of recognitions is reached, a device reset is required to resume normal operation. To perform a reset, either power cycle the device or press the SW2 button.

More information on the Sensory speech recognition library can be found here: :ref:`sln_voice_ffd_speech_recognition_sensory`.

More information on the Cyberon speech recognition library can be found here: :ref:`sln_voice_ffd_speech_recognition_cyberon`

