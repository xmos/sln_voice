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
   software_desc/power
   software_desc/src

Overview
========

The estimated power usage of the example application, while in
`POWER_STATE_FULL`, varies from 100-141 mW. This will vary based on component
tolerances and any user added code and/or user added compile options.

By default, the application will startup using a system frequency of 600 MHz
which will consume around 141 mW. After startup, `tile[1]` clock divider is
enabled and set to 3 bringing the tile's frequency down to 200 MHz, where it
will consumer around 113 mW. Tile frequencies lower than this may lead to
application instability. When the application enters `POWER_STATE_LOW`,
the `tile[0]` clock frequency will be divided by 600 and the switch clock
frequency by 30 bringing the frequencies to 1 MHz and 20 MHz, respectively. This
low power state consumes around 50 mW.

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
   * - power
     - Low power state and control
     - :ref:`sln_voice_ffd_power`
   * - src
     - Main application
     - :ref:`sln_voice_ffd_src`
