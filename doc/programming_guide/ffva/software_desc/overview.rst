.. _sln_voice_ffva_software_desc_overview:

********
Overview
********

There are three main build configurations for this application.

.. list-table:: FFVA INT Fixed Delay Resources
   :widths: 30 10 30
   :header-rows: 1
   :align: left

   * - Resource
     - Tile 0
     - Tile 1
   * - Total Memory Free
     - 141k
     - 80k
   * - Runtime Heap Memory Free
     - 75k
     - 76k

.. list-table:: FFVA INT Cyberon Fixed Delay Resources
   :widths: 30 10 30
   :header-rows: 1
   :align: left

   * - Resource
     - Tile 0
     - Tile 1
   * - Total Memory Free
     - 21k
     - 79k
   * - Runtime Heap Memory Free
     - 19k
     - 81k

.. list-table:: FFVA UA ADEC Resources
   :widths: 30 10 30
   :header-rows: 1
   :align: left

   * - Resource
     - Tile 0
     - Tile 1
   * - Total Memory Free
     - 94k
     - 59k
   * - Runtime Heap Memory Free
     - 54k
     - 83k

The description of the software is split up by folder:

.. list-table:: FFVA Software Description
   :widths: 40 120
   :header-rows: 1
   :align: left

   * - Folder
     - Description
   * - :ref:`sln_voice_ffva_audio_pipeline`
     - Preconfigured audio pipelines
   * - :ref:`sln_voice_ffva_bsp_config`
     - Board support configuration setting up software based IO peripherals
   * - :ref:`sln_voice_ffva_filesystem_support`
     - Filesystem contents for application
   * - :ref:`sln_voice_ffva_src`
     - Main application
   * - :ref:`sln_voice_intent_engine`
     - Intent engine integration (FFVA INT Cyberon only)
   * - :ref:`sln_voice_intent_handler`
     - Intent engine output integration (FFVA INT Cyberon only)