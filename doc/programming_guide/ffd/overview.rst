.. include:: ../../substitutions.rst

.. _sln_voice_ffd_overview:

********
Overview
********

This is the far-field voice local command (FFD) example design with Wanson speech recognition and local dictionary.

When a wakeword phrase is detected followed by an command phrase, the application will output an audio response and a discrete message over |I2C| and UART.

This software is an evaluation version only.  It includes a mechanism that limits the maximum number of recognitions to 50. You can reset the counter to 0 by restarting or rebooting the application.  The application can be rebooted by power cycling or pressing the SW2 button.

More information on the Wanson speech recognition library can be found here: :ref:`sln_voice_Wanson`
