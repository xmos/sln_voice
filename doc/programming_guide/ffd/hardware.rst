.. include:: ../../substitutions.rst

******************
Supported Hardware
******************

This example application is supported on the `XK-VOICE-L71 <https://www.digikey.co.uk/en/products/detail/xmos/XK-VOICE-L71/15761172>`_ board.

Setting up the Hardware
=======================

This example design requires an XTAG4 and XK-VOICE-L71 board.

.. image:: ../../shared/images/all_components.jpg
  :width: 800
  :alt: all components

Optionally, an external microphone array board can be used. If an external microphone array is used, the MUTE switch must be set ON to receive data. When using an external microphone array, the onboard hardware mute circuit is always bypassed. Refer to the schematic for more details.


xTAG
----

The xTAG is used to program and debug the device

Connect the xTAG to the debug header, as shown below.

.. image:: ../../shared/images/xtag_installation.jpg
  :width: 800
  :alt: xtag

Connect the micro USB XTAG4 and micro USB XK-VOICE-L71 to the programming host.

.. image:: ../../shared/images/host_setup.jpg
  :width: 800
  :alt: programming host setup

Speakers (OPTIONAL)
-------------------

This example application features audio playback responses.  Speakers can be connected to the LINE OUT on the XK-VOICE-L71.
