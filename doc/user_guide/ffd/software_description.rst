.. _sln_voice_ffd_software_description:

####################
Software Description
####################

.. include:: <isonum.txt>

.. toctree::
   :maxdepth: 1
   :hidden:

   software_desc/bsp_config
   software_desc/ext
   software_desc/filesystem_support
   software_desc/host
   software_desc/inference
   software_desc/src

Overview
========

The estimated power usage of the example application varies from 100-141 mW.  This will vary based on component tolerances and any user added code and/or user added compile options.

By default, the application will consume around 141 mW, with a system frequency of 600 MHz.  By changing the system frequency to 400 MHz, the application will consume around 110 mW.  By changing tile 0 to 400 MHz and tile 1 to 200 MHz, the application will consume 100 mW.  Tile frequencies lower than these may lead to application instability.

.. list-table:: FFD Resources
   :widths: 30 10 30
   :header-rows: 1
   :align: left

   * - Resource
     - Tile 0
     - Tile 1
   * - Unused CPU Time (600 MHz)
     - 69%
     - 55 %
   * - Total Memory Free
     - 210k
     - 273k
   * - Runtime Heap Memory Free
     - 18k
     - 6k

The description of the software is split up by folder:

.. list-table:: FFD Software Description
   :widths: 30 100 30
   :header-rows: 1
   :align: left

   * - Folder
     - Description
     - Link
   * - bsp_config
     - Board support configuration setting up software based IO peripherals
     - :ref:`sln_voice_ffd_bsp_config`
   * - ext
     - Application extensions
     - :ref:`sln_voice_ffd_ext`
   * - filesystem_support
     - Filesystem contents for application
     - :ref:`sln_voice_ffd_filesystem_support`
   * - host
     - Host applications
     - :ref:`sln_voice_ffd_host`
   * - inference
     - Inferencing engine integration
     - :ref:`sln_voice_ffd_inference`
   * - src
     - Main application
     - :ref:`sln_voice_ffd_src`
