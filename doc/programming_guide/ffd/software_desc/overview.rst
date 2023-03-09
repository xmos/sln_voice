.. _sln_voice_ffd_software_desc_overview:

********
Overview
********

The estimated power usage of the example application, while in
`POWER_STATE_FULL`, varies from 100-141 mW. This will vary based on component
tolerances and any user added code and/or user added compile options.

By default, the application will startup using a system frequency of 600 MHz
which will consume around 141 mW. After startup, `tile[1]` clock divider is
enabled and set to 3 bringing the tile's frequency down to 300 MHz, where it
will consume around 114 mW. Tile frequencies lower than this may lead to
application instability. When the application enters `POWER_STATE_LOW`,
the `tile[0]` clock frequency will be divided by 600 and the switch clock
frequency by 30 bringing the frequencies to 1 MHz and 20 MHz, respectively. This
low power state consumes around 55 mW.

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
   * - Low Power
     - 55
   * - Full Power
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
   * - :ref:`sln_voice_ffd_power`
     - Low power state and control
