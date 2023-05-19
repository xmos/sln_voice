.. _sln_voice_ffd_software_desc_overview:

********
Overview
********

The estimated power usage of the example application varies from 100-141 mW. This will vary based on component tolerances and any user added code and/or user added compile options.

.. list-table:: FFD Resources
   :widths: 30 10 30
   :header-rows: 1
   :align: left

   * - Resource
     - Tile 0
     - Tile 1
   * - Unused CPU Time (600 MHz)
     - 83%
     - 27%
   * - Total Memory Free
     - 192k
     - 173k
   * - Runtime Heap Memory Free
     - 38k
     - 42k

.. list-table:: FFD Power Usage
   :widths: 30 30
   :header-rows: 1
   :align: left

   * - Power State
     - Power (mW)
   * - Always
     - 114

The description of the software is split up by folder:

.. list-table:: FFD Software Description
   :widths: 40 120
   :header-rows: 1
   :align: left

   * - Folder
     - Description
   * - :ref:`sln_voice_ffd_asr`
     - ASR engine ports
   * - :ref:`sln_voice_ffd_bsp_config`
     - Board support configuration setting up software based IO peripherals
   * - :ref:`sln_voice_ffd_ext`
     - Application extensions
   * - :ref:`sln_voice_ffd_filesystem_support`
     - Filesystem contents for application
   * - :ref:`sln_voice_ffd_src`
     - Main application
   * - :ref:`sln_voice_ffd_intent_engine`
     - Intent engine integration
   * - :ref:`sln_voice_ffd_intent_handler`
     - Intent engine output integration
