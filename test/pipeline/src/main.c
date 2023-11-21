// Copyright 2020-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <stdio.h>
#include <string.h>

#include <platform.h>
#include <xs1.h>
#include <xcore/channel.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"
#include "queue.h"

/* Library headers */
#include "rtos_printf.h"

/* App headers */
#include "app_conf.h"
#include "audio_pipeline.h"
#include "xscope_fileio_task.h"
#include "xscope_io_device.h"
#include "xmath/xmath.h"
#include "platform/platform_init.h"
#include "platform/driver_instances.h"

#ifndef MEM_ANALYSIS_ENABLED
#define MEM_ANALYSIS_ENABLED 0
#endif

#if appconfAUDIO_PIPELINE_SUPPORTS_TRACE
static int audio_pipeline_output_counter = 0;
static char trace_buffer[appconfOUTPUT_TRACE_SIZE_BYTES];
static trace_data_t trace_data;
#endif // appconfAUDIO_PIPELINE_SUPPORTS_TRACE

void audio_pipeline_input(void *input_app_data,
                        int32_t **input_audio_frames,
                        size_t ch_count,
                        size_t frame_count)
{
    (void) input_app_data;
    rx_audio_from_host((int8_t **)input_audio_frames, appconfINPUT_BRICK_SIZE_BYTES);
}

int audio_pipeline_output(void *output_app_data,
                        int32_t **output_audio_frames,
                        size_t ch_count,
                        size_t frame_count)
{
    (void) output_app_data;

    // Write output to host
    tx_audio_to_host((uint8_t*)output_audio_frames, appconfOUTPUT_BRICK_SIZE_BYTES);

#if appconfAUDIO_PIPELINE_SUPPORTS_TRACE
    if (output_app_data) {
        // Write trace data to host
        trace_data_t *trace_data = (trace_data_t *)output_app_data;

        sprintf(trace_buffer, "TRACE: %d,%d,%d\n", 
            audio_pipeline_output_counter++,
            Q31(trace_data->input_vnr_pred),
            trace_data->control_flag
        );
        tx_trace_to_host((int8_t*)trace_buffer, strlen(trace_buffer));
    }
#endif

    return AUDIO_PIPELINE_FREE_FRAME;
}

void vApplicationMallocFailedHook(void)
{
    rtos_printf("Malloc Failed on tile %d!\n", THIS_XCORE_TILE);
    xassert(0);
    for(;;);
}

#if MEM_ANALYSIS_ENABLED
static void mem_analysis(void)
{
    for (;;) {
        rtos_printf("Tile[%d]:\n\tMinimum heap free: %d\n\tCurrent heap free: %d\n", THIS_XCORE_TILE, xPortGetMinimumEverFreeHeapSize(), xPortGetFreeHeapSize());
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
#endif

void startup_task(void *arg)
{
    rtos_printf("Startup task running from tile %d on core %d\n", THIS_XCORE_TILE, portGET_CORE_ID());

    platform_start();

    // need to give the platform a moment to start
    vTaskDelay(pdMS_TO_TICKS(1000));

#if ON_TILE(XSCOPE_HOST_IO_TILE)
    /* Wait until xscope_fileio is initialized */
    while(xscope_fileio_is_initialized() == 0) {
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    xscope_fileio_tasks_create(appconfXSCOPE_IO_TASK_PRIORITY, NULL);
#endif

    // need to give the xscope_fileio tasks a moment to start
    vTaskDelay(pdMS_TO_TICKS(1000));

#if ((appconfAUDIO_PIPELINE_INPUT_TILE_NO == 0) || (appconfAUDIO_PIPELINE_OUTPUT_TILE_NO == 0)) && ON_TILE(0)
    rtos_printf("Initializing audio pipeline on tile 0\n");

#if appconfAUDIO_PIPELINE_SUPPORTS_TRACE
    audio_pipeline_init(NULL, &trace_data);
#else
    audio_pipeline_init(NULL, NULL);
#endif // appconfAUDIO_PIPELINE_SUPPORTS_TRACE

#endif

#if ((appconfAUDIO_PIPELINE_INPUT_TILE_NO == 1) || (appconfAUDIO_PIPELINE_OUTPUT_TILE_NO == 1)) && ON_TILE(1)
    rtos_printf("Initializing audio pipeline on tile 1\n");

#if appconfAUDIO_PIPELINE_SUPPORTS_TRACE
    audio_pipeline_init(NULL, &trace_data);
#else
    audio_pipeline_init(NULL, NULL);
#endif // appconfAUDIO_PIPELINE_SUPPORTS_TRACE

#endif

#if MEM_ANALYSIS_ENABLED
    mem_analysis();
#else
    vTaskSuspend(NULL);
    while(1){;} /* Trap */
#endif
}

void vApplicationMinimalIdleHook(void)
{
    rtos_printf("idle hook on tile %d core %d\n", THIS_XCORE_TILE, rtos_core_id_get());
    asm volatile("waiteu");
}

static void tile_common_init(chanend_t c)
{
    platform_init(c);
    chanend_free(c);

    xTaskCreate((TaskFunction_t) startup_task,
                "startup_task",
                RTOS_THREAD_STACK_SIZE(startup_task),
                NULL,
                appconfSTARTUP_TASK_PRIORITY,
                NULL);

    rtos_printf("start scheduler on tile %d\n", THIS_XCORE_TILE);
    vTaskStartScheduler();
}

#if ON_TILE(0)
void main_tile0(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
{
    (void) c0;
    (void) c2;
    (void) c3;

    tile_common_init(c1);
}
#endif

#if ON_TILE(1)
void main_tile1(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
{
    (void) c1;
    (void) c2;
    (void) c3;

    tile_common_init(c0);
}
#endif
