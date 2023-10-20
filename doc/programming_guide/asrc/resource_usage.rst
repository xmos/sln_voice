
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
