// Copyright (c) 2020-2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

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
#include "platform/platform_init.h"
#include "platform/driver_instances.h"

void audio_pipeline_input(void *input_app_data,
                        int32_t **input_audio_frames,
                        size_t ch_count,
                        size_t frame_count)
{
    (void) input_app_data;
    rx_from_host((int8_t **)input_audio_frames, appconfINPUT_BRICK_SIZE_BYTES);
}

int audio_pipeline_output(void *output_app_data,
                        int32_t **output_audio_frames,
                        size_t ch_count,
                        size_t frame_count)
{
    (void) output_app_data;

    tx_to_host((uint8_t*)output_audio_frames, appconfOUTPUT_BRICK_SIZE_BYTES);
    return AUDIO_PIPELINE_FREE_FRAME;
}

void vApplicationMallocFailedHook(void)
{
    rtos_printf("Malloc Failed on tile %d!\n", THIS_XCORE_TILE);
    xassert(0);
    for(;;);
}

// static void mem_analysis(void)
// {
//     for (;;) {
//         rtos_printf("Tile[%d]:\n\tMinimum heap free: %d\n\tCurrent heap free: %d\n", THIS_XCORE_TILE, xPortGetMinimumEverFreeHeapSize(), xPortGetFreeHeapSize());
//         vTaskDelay(pdMS_TO_TICKS(5000));
//     }
// }

void startup_task(void *arg)
{
    rtos_printf("Startup task running from tile %d on core %d\n", THIS_XCORE_TILE, portGET_CORE_ID());

    platform_start();

    // need to give the platform a moment to start
    vTaskDelay(pdMS_TO_TICKS(1000));

#if ON_TILE(XSCOPE_HOST_IO_TILE)
    xscope_fileio_tasks_create(appconfXSCOPE_IO_TASK_PRIORITY, NULL);
#endif

    // need to give the xscope_fileio tasks a moment to start
    vTaskDelay(pdMS_TO_TICKS(1000));

#if appconfAUDIO_PIPELINE_USES_TILE_0 && ON_TILE(0)
    rtos_printf("Initializing audio pipeline on tile 0\n");
    audio_pipeline_init(NULL, NULL);
#endif
#if appconfAUDIO_PIPELINE_USES_TILE_1 && ON_TILE(1)
    rtos_printf("Initializing audio pipeline on tile 1\n");
    audio_pipeline_init(NULL, NULL);
#endif

    //mem_analysis();
    vTaskSuspend(NULL);
    while(1){;} /* Trap */
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
