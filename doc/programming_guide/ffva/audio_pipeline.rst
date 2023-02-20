##############
Audio Pipeline
##############

========
Overview
========

The audio pipeline in FFVA processes two channel pdm microphone input into a single output channel, intended for use by an ASR engine.

The audio pipeline consists of 4 stages.

.. list-table:: FFVA Audio Pipeline
   :widths: 20 80 20 20
   :header-rows: 1
   :align: left

   * - Stage
     - Description
     - Input Channel Count
     - Output Channel Count
   * - 1
     - Acoustic Echo Cancellation (AEC)
     - 2
     - 2
   * - 2
     - Interference Canceller and Voice Activity Detection
     - 2
     - 1
   * - 3
     - Noise Suppression
     - 1
     - 1
   * - 4
     - Automatic Gain Control
     - 1
     - 1

See the Voice Framework User Guide for more information.

.. raw:: latex

    \clearpage
