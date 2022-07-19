.. _sln_voice_Wanson:

#######################
Wanson Speech AI Engine
#######################

.. include:: <isonum.txt>



License
=======
Warning and link to license


Overview
========

what is it
how it is provided
specs (memory, MIPS etc)


Not sure if there is any info on how to include the wanson module in your design, but we should have that - and it ought to be a standard interface that makes it simple for someone to replace with their own.


State machine
=============
commands vs keywords
state machine diagram

external signalling based on state



Dictionary command table
========================

.. list-table:: English langauge demo
   :widths: 100 50 50
   :header-rows: 1
   :align: left

   * - Utterances
     - Type
     - Return code
   * - Hello XMOS
     - keyword
     - tbd
   * - Hello Wanson
     - keyword
     - tbd
   * - Switch on the TV
     - command
     - tbd
   * - Channel up
     - command
     - tbd
   * - Channel down
     - command
     - tbd
   * - Volume up
     - command
     - tbd
   * - Volume down
     - command
     - tbd
   * - Switch off the TV
     - command
     - tbd
   * - Switch on the lights
     - command
     - tbd
   * - Brightness up
     - command
     - tbd
   * - Brightness down
     - command
     - tbd
   * - Switch off the lights
     - command
     - tbd
   * - Switch on the fan
     - command
     - tbd
   * - Speed up the fan
     - command
     - tbd
   * - Slow down the fan
     - command
     - tbd
   * - Set higher temperature
     - command
     - tbd
   * - Set lower temperature
     - command
     - tbd
   * - Switch off the fan
     - command
     - tbd
