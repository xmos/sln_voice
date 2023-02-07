.. _sln_voice_ffva_software_description:

####################
Software Description
####################

.. include:: <isonum.txt>

.. toctree::
   :maxdepth: 1
   :hidden:

   software_desc/bsp_config
   software_desc/filesystem_support
   software_desc/audio_pipeline
   software_desc/src

Overview
========

There are two main build configurations for this application.

.. list-table:: FFVA INT Fixed Delay Resources
   :widths: 30 10 30
   :header-rows: 1
   :align: left

   * - Resource
     - Tile 0
     - Tile 1
   * - Unused CPU Time (600 MHz)
     - 98%
     - 75%
   * - Total Memory Free
     - 166k
     - 82k
   * - Runtime Heap Memory Free
     - 75k
     - 82k

.. list-table:: FFVA UA ADEC Resources
   :widths: 30 10 30
   :header-rows: 1
   :align: left

   * - Resource
     - Tile 0
     - Tile 1
   * - Unused CPU Time (600 MHz)
     - 83%
     - 45%
   * - Total Memory Free
     - 123k
     - 58k
   * - Runtime Heap Memory Free
     - 54k
     - 83k

The description of the software is split up by folder:

.. list-table:: FFVA Software Description
   :widths: 30 100 30
   :header-rows: 1
   :align: left

   * - Folder
     - Description
     - Link
   * - audio_pipeline
     - Preconfigured audio piplines
     - :ref:`sln_voice_ffva_audio_pipeline`
   * - bsp_config
     - Board support configuration setting up software based IO peripherals
     - :ref:`sln_voice_ffva_bsp_config`
   * - filesystem_support
     - Filesystem contents for application
     - :ref:`sln_voice_ffva_filesystem_support`
   * - src
     - Main application
     - :ref:`sln_voice_ffva_src`
