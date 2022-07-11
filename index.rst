.. _sln_voice_user_guide:

###########################
XCORE |reg| VOICE Solutions
###########################

.. include:: <isonum.txt>
.. |I2C| replace:: I\ :sup:`2`\ C
.. |I2S| replace:: I\ :sup:`2`\ S
.. toctree::
   :maxdepth: 3
   :hidden:
   
   doc/ffd
   doc/stlp
   xcore_sdk/index



XMOS Solutions are a combination of the XCORE_SDK and Reference Designs, ready to run on our development kits, and targetting a range of voice assistant and voice control applications.


On GitHub
---------

Get the latest version from `sln_voice <https://github.com/xmos/sln_voice>`_

Follow the **READ_ME** how to clone this repo.

Checkout the tagged versions for the latest stable release.


Reference Designs
-----------------

.. |XK_VOICE_L71_image| image:: doc/XK_VOICE_L71.jpg
  :width: 80
  :alt: XK_VOICE_L71

.. table:: Voice Solutions - reference designs
   :align: center
   :widths: 10 50 30 10 
  
   +-------------------------+------------------------------------------------------------------------+--------------------------------------------------------------------------------------------+---------------------------------------------------------------------------+
   | Title                   | Description                                                            | Development kit                                                                            | Go to the code                                                            |
   +=========================+========================================================================+============================================================================================+===========================================================================+
   | :ref:`sln_voice_FFD`    | 2-microphone far-field voice control with |I2C| or UART interface.     | `XK-VOICE-L71 <https://www.digikey.co.uk/en/products/detail/xmos/XK-VOICE-L71/15761172>`_  | `ffd <https://github.com/xmos/sln_voice/tree/develop/applications/ffd>`_  |
   |                         | Audio pipeline includes interference cancelling and noise supression.  |                                                                                            |                                                                           |
   |                         | 25-phrase English language voice recognition.                          | |XK_VOICE_L71_image|                                                                       |                                                                           |
   +-------------------------+------------------------------------------------------------------------+--------------------------------------------------------------------------------------------+---------------------------------------------------------------------------+
   | :ref:`sln_voice_STLP`   | 2-microphone Far-field voice assistant front-end.                      | `XK-VOICE-L71 <https://www.digikey.co.uk/en/products/detail/xmos/XK-VOICE-L71/15761172>`_  | `ffd <https://github.com/xmos/sln_voice/tree/develop/applications/stlp>`_ |
   |                         | Audio pipeline includes echo cancelaation, interference cancelling and |                                                                                            |                                                                           |
   |                         | noise supression.                                                      | |XK_VOICE_L71_image|                                                                       |                                                                           |
   |                         | Stereo reference input and voice assitant output each supported        |                                                                                            |                                                                           |
   |                         | as  |I2C| or USB (UAC2.)                                               |                                                                                            |                                                                           |
   +-------------------------+------------------------------------------------------------------------+--------------------------------------------------------------------------------------------+---------------------------------------------------------------------------+



Development Tools
-----------------

Download and install the XCore `XTC Tools <https://www.xmos.ai/software-tools/>`_ version 15.1.0 or newer. If you already have the XTC Toolchain installed, you can check the version with the following command:

    xcc --version


XMOS Devices
------------

The XK-VOICE-L71 is based on the: `XU316-1024-QF60A <https://www.xmos.ai/download/XU316-1024-QF60A-xcore.ai-Datasheet(22).pdf>`_

Learn more about the `The XMOS XS3 Architecture <https://www.xmos.ai/download/The-XMOS-XS3-Architecture(5).pdf>`_


Licensing
---------

This Software is subject to the terms of the `XMOS Public Licence: Version 1 <https://github.com/xmos/xcore_sdk/blob/develop/LICENSE.rst>`_

Additional third party copyrighted code is included under the following licenses:

The Wanson speech recognition library is Copyright 2022. Shanghai Wanson Electronic Technology Co.Ltd ("WANSON") and is subject to the `Wanson Restrictive License <https://github.com/xmos/sln_voice/tree/develop/applications/ffd/inference/wanson/lib/LICENSE.md>`_

