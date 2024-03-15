
****************
Host Integration
****************

This example design can be integrated with existing solutions or modified to be a single controller solution.


Out of the Box Integration
==========================

Out of the box integration varies based on configuration.

INT requires |I2S| connections to the host.  Refer to the schematic, connecting the host reference audio playback to the ADC |I2S| and the host input audio to the DAC |I2S|.  Out of the box, the INT configuration requires an externally generated MCLK of 12.288 MHz.  24.576 MHz is also supported and can be changed via the compile option MIC_ARRAY_CONFIG_MCLK_FREQ, found in ffva_int.cmake.

UA requires a USB connection to the host.


Support for ASR engine
======================

The ``example_ffva_int_cyberon_fixed_delay`` provides an example about how to include an ASR engine, the  Cyberon DSPotter™.

All the considerations made in the section about FFD are still valid for the FFVA example. The only notable difference is that the pipeline output in the FFVA example
is on the same tile as the ASR engine, i.e. tile 0.
.. note::

    Both the audio pipeline and the ASR engine process use the same sample block length. ``appconfINTENT_SAMPLE_BLOCK_LENGTH`` and ``appconfAUDIO_PIPELINE_FRAME_ADVANCE`` are both 240.

More information about the Cyberon engine can be found in  :ref:`the Speech Recognition - Cyberon <sln_voice_ffd_speech_recognition_cyberon>`_ section.

|newpage|

*******************
Design Architecture
*******************

The application consists of a PDM microphone input which is fed through the XMOS-VOICE DSP blocks.  The output ASR channel is then output over |I2S| or USB.

.. figure:: diagrams/ffva_diagram.drawio.png
   :align: center
   :scale: 80 %
   :alt: ffva diagram

|newpage|
