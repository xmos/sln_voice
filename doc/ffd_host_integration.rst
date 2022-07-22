.. _sln_voice_ffd_host_integration:

#######################
Host Integration
#######################

.. include:: <isonum.txt>



Overview
========

This section describes the connections that would need to be made to an external host for plug and play integration with existing devices.

When an intent is found, the XCORE device will check if the host is awake, by checking the Host Status GPIO pin.  If the host is awake the intent code will be transmitted over I2C and/or UART.

If the host is not awake, the XCORE device will trigger a transition of the Wakeup GPIO pin.  This can be configured to be a rising or falling edge. The XCORE device will then wait a compile time set delay before transmitting the intent over the I2C and/or UART interface.

.. |ffd_host_integration_diagram_image| figure:: diagrams/ffd_host_integration_diagram.drawio.png
   :align: center
   :scale: 80 %
   :alt: ffd host integration diagram

UART
^^^^
.. list-table:: UART Connections
   :widths: 50 50
   :header-rows: 1
   :align: left

   * - FFD Connection
     - Host Connection
   * - J4:24
     - UART RX
   * - J4:20
     - GND

I2C
^^^
.. list-table:: I2C Connections
   :widths: 50 50
   :header-rows: 1
   :align: left

   * - FFD Connection
     - Host Connection
   * - J4:3
     - SDA
   * - J4:5
     - SCL
   * - J4:9
     - GND

GPIO
^^^^
.. list-table:: GPIO Connections
   :widths: 50 50
   :header-rows: 1
   :align: left

   * - FFD Connection
     - Host Connection
   * - J4:19
     - Wake up input
   * - J4:21
     - Host Status output
