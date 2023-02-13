.. _sln_voice_ffd_inference:

#########
inference
#########

.. include:: <isonum.txt>



Overview
========

This folder contains inferencing modules for the FFD application.

.. list-table:: FFD inference
   :widths: 30 50
   :header-rows: 1
   :align: left

   * - Filename/Directory
     - Description
   * - api directory
     - include folder for inferencing modules
   * - wanson directory
     - contains the Wanson engine and associated port code
   * - inference.cmake
     - cmake for adding inference target


Major Components
================

The inference module provides the application with two API functions:

.. code-block:: c
    :caption: Intent API (intent_engine.h)

    int32_t intent_engine_create(uint32_t priority, void *args);
    int32_t intent_engine_sample_push(int32_t *buf, size_t frames);

If replacing the existing model, these are the only two functions that are required to be populated.


intent_engine_create
^^^^^^^^^^^^^^^^^^^^^^^

This function has the role of creating the model running task and providing a pointer, which can be used by the application to handle the output intent result.  In the case of the Wanson model, the application provides a FreeRTOS Queue object.

In FFD, the audio pipeline output is on tile 1 and the Wanson engine on tile 0.

.. code-block:: c
    :caption: intent_engine_create snippet (wanson_inf_eng_port.c)

    #if ASR_TILE_NO == AUDIO_PIPELINE_TILE_NO
        wanson_engine_task_create(priority);
    #else
        wanson_engine_intertile_task_create(priority);
    #endif

The call to wanson_engine_intertile_task_create() will create two threads on tile 0.  One thread is the Wanson engine thread.  The other thread is an intertile rx thread, which will interface with the audiopipeline output.


intent_engine_sample_push
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This function has the role of sending the ASR output channel from the audiopipeline to the inference engine.

In FFD, the audio pipeline output is on tile 1 and the Wanson engine on tile 0.

.. code-block:: c
    :caption: intent_engine_create snippet (wanson_inf_eng_port.c)

    #if ASR_TILE_NO == AUDIO_PIPELINE_TILE_NO
        wanson_engine_samples_send_local(
                frames,
                buf);
    #else
        wanson_engine_samples_send_remote(
                intertile_ctx,
                frames,
                buf);
    #endif

The call to wanson_engine_samples_send_remote() will send the audio samples to the previously configured intertile rx thread.


wanson_engine_proc_keyword_result
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This weak function can be overridden by the application to handle the intent in a completely different manner.
