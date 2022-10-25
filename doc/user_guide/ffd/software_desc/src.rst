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
   * - power directory
     - contains low power state and control code
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


Audio Pipeline
==============

The audio pipeline module provides the application with three API functions:

.. code-block:: c
    :caption: Audio Pipeline API (audio_pipeline.h)

    void audio_pipeline_init(
            void *input_app_data,
            void *output_app_data);

    void audio_pipeline_input(
            void *input_app_data,
            int32_t **input_audio_frames,
            size_t ch_count,
            size_t frame_count);

    int audio_pipeline_output(
            void *output_app_data,
            int32_t **output_audio_frames,
            size_t ch_count,
            size_t frame_count);


audio_pipeline_init
^^^^^^^^^^^^^^^^^^^

This function has the role of creating the audio pipeline, with two optional application pointers which are provided to the application in the audio_pipeline_input() and audio_pipeline_output() callbacks.

In FFD, the audio pipeline is initialized with no additional arguments, and instantiates a 3 stage pipeline on tile 1, as described in:
:ref:`sln_voice_ffd_ap`


audio_pipeline_input
^^^^^^^^^^^^^^^^^^^^

This function has the role of providing the audio pipeline with the input frames.

This function is weak so the application can override it if desired.

In FFD, the input is received from the rtos_mic_array driver.


audio_pipeline_output
^^^^^^^^^^^^^^^^^^^^^

This function has the role of receiving the processed audio pipeline output.

This function is weak so the application can override it if desired.

In FFD, the output is sent to the inference engine. If `appconfLOW_POWER_ENABLED`
is set true, then the output will be dropped if the power state is not
`POWER_STATE_FULL`. In certain conditions and environments, this behavior may
cause the wake word to be missed. Further adjustments to the application
configuration settings related to the VNR low power thresholds may mitigate such
issues. See :ref:`sln_voice_ffd_power`.


Main
====

The major components of main are:

.. code-block:: c
    :caption: Main components (main.c)

    void startup_task(void *arg)
    void vApplicationMinimalIdleHook(void)
    void tile_common_init(chanend_t c)
    void main_tile0(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
    void main_tile1(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)


startup_task
^^^^^^^^^^^^

This function has the role of launching tasks on each tile.  For those familiar with XCORE, it is comparable to the main par loop in an XC main.


vApplicationMinimalIdleHook
^^^^^^^^^^^^^^^^^^^^^^^^^^^

This is a FreeRTOS callback.  By calling "waiteu" without events configured, this has the effect of both MIPs and power savings on XCORE.

.. code-block:: c
    :caption: vApplicationMinimalIdleHook (main.c)

    asm volatile("waiteu");

tile_common_init
^^^^^^^^^^^^^^^^

This function is the common tile initialization, which initializes the bsp_config, creates the startup task, and starts the FreeRTOS kernel.


main_tile0
^^^^^^^^^^

This function is the application C entry point on tile 0, provided by the SDK.


main_tile1
^^^^^^^^^^

This function is the application C entry point on tile 1, provided by the SDK.
