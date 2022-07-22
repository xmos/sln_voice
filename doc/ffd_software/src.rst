.. _sln_voice_ffd_src:

###
src
###

.. include:: <isonum.txt>



Overview
========

This folder contains the core application source.

.. list-table:: FFD src
   :widths: 30 50
   :header-rows: 1
   :align: left

   * - Filename/Directory
     - Description
   * - audio_pipeline directory
     - contains example XMOS audio pipeline
   * - gpio_ctrl directory
     - contains general purpose input handling task and LED output heartbeat task
   * - intent_handler directory
     - contains intent handling code
   * - rtos_conf directory
     - contains default FreeRTOS configuration header
   * - ssd1306
     - contains code for configuring and controlling the optional ssd1306 display daughter board
   * - app_conf_check.h
     - header to validate app_conf.h
   * - app_conf.h
     - header to describe app configuration
   * - config.xscope
     - xscope configuration file
   * - ff_appconf.h
     - default fatfs configuration header
   * - main.c
     - main application source file
   * - xcore_device_memory.c
     - model loading from filesystem source file
   * - xcore_device_memory.h
     - model loading from filesystem header file
