.. include:: ../../substitutions.rst

********
Overview
********

This is the XMOS far-field voice local command (FFD) reference design with Wanson speech recognition and local dictionary.

When a wakeup phrase is followed by an intent phrase the application will output an audio response, |I2C|, and uart discrete message.

This software is an evaluation version only.  It includes a mechanism that limits the maximum number of recognitions to 50. You can reset the counter to 0 by restarting or rebooting the application.  The application can be rebooted by power cycling or pressing the SW2 button.

More information on the Wanson speech recognition library can be found here: :ref:`sln_voice_Wanson`
