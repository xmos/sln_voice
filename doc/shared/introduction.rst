*******************
Product Description 
*******************

The XCORE-VOICE Solution consists of example designs and a C-based SDK for the development of audio front-end applications to support far-field voice use cases on the xcore.ai family of chips (XU316). The XCORE-VOICE design is currently based on FreeRTOS, leveraging the flexibility of the xcore.ai platform and providing designers with a familiar environment to customize and develop products.

XCORE-VOICE example designs provide turn-key solutions to enable easier product development for smart home applications such as light switches, thermostats, and home appliances. xcore.ai’s unique architecture providing powerful signal processing and accelerated AI capabilities combined with the XCORE-VOICE framework allows designers to incorporate keyword, event detection, or advanced local dictionary support to create a complete voice interface solution. 

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
