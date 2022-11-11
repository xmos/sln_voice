.. include:: <isonum.txt>
.. include:: ../substitutions.rst

.. _sln_voice_datasheet:

#########
DATASHEET
#########

************
Key Features 
************

The XCORE-VOICE Solution takes advantage of the flexible software-defined xcore-ai architecture to support numerous far-field voice use cases through the available example designs and the ability to construct user-defined audio pipeline from the SW components and libraries in the C-based SDK. 

These include: 

**Voice Processing components**

- Two PDM microphone interfaces 
- Digital signal processing pipeline 
- Full duplex, stereo, Acoustic Echo Cancellation (AEC) 
- Reference audio via I2S with automatic bulk delay insertion 
- Point noise suppression via interference canceller 
- Switchable stationary noise suppressor 
- Programmable Automatic Gain Control (AGC) 
- Flexible audio output routing and filtering 
- Independent audio paths for communications and Automatic Speech Recognition (ASR) 
- Support for Wanson Speech Recognition or chooser-0defined 3rd party ASR 

**Device Interface components**

- Full speed USB2.0 compliant device supporting USB Audio Class (UAC) 2.0 
- Flexible Peripheral Interfaces 
- Programmable digital general-purpose inputs and outputs 

**Example Designs utilising above components**

- Far-Field Voice Local Control 
- Far-Field Voice Assistance 

**Firmware Management**

- Boot from QSPI Flash 
- Default firmware image for power-on operation 
- Option to boot from a local host processor via SPI 
- Device Firmware Update (DFU) via USB or other transport

**Power Consumption**

- Typical power consumption 300-350mW 
- Low power modes down to 100mW 

****************
Voice Processing
****************

TODO: Complete this section with an overview of the voice processing components.

*****************
Device Interfaces
*****************

TODO: Complete this section with the options for interfaces.

********
Hardware
********

TODO: Complete this section with more info on hardware and links to hardware datasheets.

**********************
Copyright & Disclaimer
**********************

Copyright © 2022 XMOS Ltd, All Rights Reserved.

XMOS Ltd is the owner or licensee of this design, code, or Information (collectively, the “Information”) and is providing it to you “AS IS” with no warranty of any kind, express or implied and shall have no liability in relation to its use. XMOS Ltd makes no representation that the Information, or any particular implementation thereof, is or will be free from any claims of infringement and again, shall have no liability in relation to any such claims.

XMOS, XCORE-VOICE and the XMOS logo are registered trademarks of XMOS Ltd in the United Kingdom and other countries and may not be used without written permission. Company and product names mentioned in this document are the trademarks or registered trademarks of their respective owners.
