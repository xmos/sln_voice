.. include:: <isonum.txt>
.. include:: ../substitutions.rst

.. _sln_voice_product_description:

###################
PRODUCT DESCRIPTION
###################

The XCORE-VOICE Solution consists of example designs and a C-based SDK for the development of audio front-end applications to support far-field voice use cases on the xcore.ai family of chips (XU316). The XCORE-VOICE design is currently based on FreeRTOS, leveraging the flexibility of the xcore.ai platform and providing designers with a familiar environment to customize and develop products.

XCORE-VOICE example designs provide turn-key solutions to enable easier product development for smart home applications such as light switches, thermostats, and home appliances. xcore.ai’s unique architecture providing powerful signal processing and accelerated AI capabilities combined with the XCORE-VOICE framework allows designers to incorporate keyword, event detection, or advanced local dictionary support to create a complete voice interface solution. 

**********************
Obtaining the Hardware
**********************

The XK-VOICE-L71 is based on the: `XU316-1024-QF60A <https://www.xmos.ai/file/xu316-1024-qf60b-xcore_ai-datasheet?version=latest>`_

Learn more about the `The XMOS XS3 Architecture <https://www.xmos.ai/download/The-XMOS-XS3-Architecture.pdf>`_

**********************
Obtaining the Software
**********************

-----------------
Development Tools
-----------------

Download and install the XCore `XTC Tools <https://www.xmos.ai/software-tools/>`_ version 15.1.4 or newer. If you already have the XTC Toolchain installed, you can check the version with the following command:

    xcc --version

Clone, build and install the `XCORE SDK host application tools <https://www.xmos.ai/documentation/XM-014660-PC-LATEST/html/doc/quick_start/installation.html#step-2-install-host-applications>`__. 

--------------------
XCORE-VOICE Examples
--------------------

Get the latest version from `sln_voice <https://github.com/xmos/sln_voice>`_

Follow the **README** how to clone this repo.

Checkout the tagged versions for the latest stable release.

***************
Example Designs
***************

.. image:: ../images/XK_VOICE_L71.jpg
  :width: 300
  :alt: XK_VOICE_L71

.. list-table:: Voice Solutions - example designs
    :widths: 50 100 100 100
    :header-rows: 1
    :align: left

    * - Title
      - Description
      - Development kit
      - Go to the code
    * - :ref:`sln_voice_FFD` 
      - - 2-microphone far-field voice control with |I2C| or UART interface 
        - Audio pipeline includes interference cancelling and noise supression 
        - 25-phrase English language voice recognition
      - `XK-VOICE-L71 <https://www.digikey.co.uk/en/products/detail/xmos/XK-VOICE-L71/15761172>`__
      - `examples/ffd <https://github.com/xmos/sln_voice/tree/develop/examples/ffd>`__
    * - :ref:`sln_voice_STLP`
      - - 2-microphone far-field voice assistant front-end
        - Audio pipeline includes echo cancelation, interference cancelling and noise suppression
        - Stereo reference input and voice assistant output each supported as |I2C| or USB (UAC2.0)    
      - `XK-VOICE-L71 <https://www.digikey.co.uk/en/products/detail/xmos/XK-VOICE-L71/15761172>`__
      - `examples/stlp <https://github.com/xmos/sln_voice/tree/develop/examples/stlp>`__
