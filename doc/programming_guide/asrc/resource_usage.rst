
**************
Resource Usage
**************

Memory
======

Tile 0 memory usage in bytes:

.. code-block:: console

    Memory available:       524288,   used:      260976+.  MAYBE
    (Stack: 5556+, Code: 70008, Data: 185412)

Tile 1 memory usage in bytes:

.. code-block:: console

    Memory available:       524288,   used:      206180 .  OKAY
    (Stack: 4356, Code: 37316, Data: 164508)

Chanends
========

This application uses 19 chanends on the USB tile (tile 0) and 11 chanends on the I2S tile (tile 1)

The chanend use breakup is as follows:

Tile 0
------

.. list-table:: Tile 0 chanend usage
   :widths: 30 50
   :header-rows: 1
   :align: left

   * - Resource
     - Chanends used
   * - RTOS scheduler
     - 5 (one per bare-metal core dedicated to RTOS)
   * - RTOS USB driver
     - 10 (2 per endpoint, per direction. 2 for SOF input)
   * - :ref:`intertile-context-label`
     - 3
   * - xscope
     - 1


Tile 1
------

.. list-table:: Tile 1 chanend usage
   :widths: 30 50
   :header-rows: 1
   :align: left

   * - Resource
     - Chanends used
   * - RTOS scheduler
     - 5 (one per bare-metal core dedicated to RTOS)
   * - RTOS |I2S| driver
     - 2
   * - :ref:`intertile-context-label`
     - 3
   * - xscope
     - 1


.. _intertile-context-label:

Intertile contexts
------------------

The application uses 3 intertile contexts for cross tile communication.

    * A dedicated intertile context for sending ASRC output data from the I2S tile to the USB tile.
    * A dedicated intertile context for sending ASRC output data from the USB tile to the I2S tile.
    * The intertile context for all other cross tile communication.


CPU
===

Profiling the CPU usage for this application using an RTOS friendly profiling tool is still TBD.
However, I have profiled some application tasks as well as gathered some already existing profiling numbers for the drivers and listed them in the table below.
Each tile has 5 bare-metal cores being used for running RTOS tasks so each core has a fixed bandwidth of 120MHz available.

Tile 0
------

.. list-table:: Tile 0 tasks MIPS
   :widths: 50 50
   :header-rows: 1
   :align: left

   * - RTOS Task
     - MIPS
   * - XUD
     - 120 (from :ref:`table-CPU-sln-voice`)
   * - ASRC in the USB -> ASRC -> |I2S| path for the worst case of 48KHz to 192KHz upsampling
     - 85
   * - usb_task
     - 24
   * - i2s_to_usb_intertile
     - 14



Tile 1
------

.. list-table:: Tile 1 tasks MIPS
   :widths: 50 50
   :header-rows: 1
   :align: left

   * - RTOS Task
     - MIPS
   * - |I2S| Slave
     - 96 (from :ref:`table-CPU-sln-voice`)
   * - ASRC in the |I2S| -> ASRC -> USB path for the worst case of 192KHz to 48KHz downsampling
     - 75
   * - usb_to_i2s_intertile
     - 0.7
   * - rate_server
     - 19



