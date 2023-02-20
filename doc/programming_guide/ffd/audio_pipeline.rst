.. _sln_voice_ffd_ap:

##############
Audio Pipeline
##############

.. include:: <isonum.txt>



Overview
========

The audio pipeline in FFD processes two channel pdm microphone input into a single output channel, intended for use by an ASR engine.

The audio pipeline consists of 3 stages.

.. list-table:: FFD Audio Pipeline
   :widths: 30 100 10 10
   :header-rows: 1
   :align: left

   * - Stage
     - Description
     - Input Channel Count
     - Output Channel Count
   * - 1
     - Interference Canceller and Voice Activity Detection
     - 2
     - 1
   * - 2
     - Noise Suppression
     - 1
     - 1
   * - 3
     - Automatic Gain Control
     - 1
     - 1

See the Voice Framework User Guide for more information. 
