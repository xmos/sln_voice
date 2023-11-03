// Copyright 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>
#include <xcore/channel.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"
#include "queue.h"
#include "event_groups.h"

/* Library headers */
#include "rtos_printf.h"
#include "rtos_clock_control.h"

/* App headers */
#include "app_conf.h"
#include "platform/platform_conf.h"
#include "platform/platform_init.h"
#include "platform/driver_instances.h"
#include "audio_pipeline.h"
#include "intent_engine/intent_engine.h"
#include "wakeword/wakeword.h"
#include "fs_support.h"
#include "gpio_ctrl/gpi_ctrl.h"
#include "gpio_ctrl/leds.h"
#include "intent_handler/intent_handler.h"
#include "power/power_state.h"
#include "power/power_control.h"
#include "power/low_power_audio_buffer.h"

#ifndef MEM_ANALYSIS_ENABLED
#define MEM_ANALYSIS_ENABLED 0
#endif

void startup_task(void *arg);
void tile_common_init(chanend_t c);

#if ON_TILE(MICARRAY_TILE_NO)

static void no_preempt_task(void *arg)
{
    vTaskPreemptionDisable(NULL);

    while(1) {
        asm volatile("waiteu");
    }
}

static void no_preempt_task_create(void)
{
    // This task serves to prevent other tasks from being placed on appconfPDM_MIC_INTERRUPT_CORE
    static TaskHandle_t task_handle;
    xTaskCreate((TaskFunction_t)no_preempt_task,
            "no_preempt_task",
            RTOS_THREAD_STACK_SIZE(no_preempt_task),
            NULL,
            (configMAX_PRIORITIES - 1),
            &task_handle);

    UBaseType_t uxCoreAffinityMask = 1 << appconfPDM_MIC_INTERRUPT_CORE;
    vTaskCoreAffinitySet(task_handle, uxCoreAffinityMask);
}

#endif

void audio_pipeline_input(void *input_app_data,
                          int32_t **input_audio_frames,
                          size_t ch_count,
                          size_t frame_count)
{
    (void) input_app_data;

    static int flushed;
    while (!flushed) {
        size_t received;
        received = rtos_mic_array_rx(mic_array_ctx,
                                     input_audio_frames,
                                     frame_count,
                                     0);
        if (received == 0) {
            rtos_mic_array_rx(mic_array_ctx,
                              input_audio_frames,
                              frame_count,
                              portMAX_DELAY);
            flushed = 1;
        }
    }

    rtos_mic_array_rx(mic_array_ctx,
                      input_audio_frames,
                      frame_count,
                      portMAX_DELAY);
}

int audio_pipeline_output(void *output_app_data,
                          int32_t **output_audio_frames,
                          size_t ch_count,
                          size_t frame_count)
{
#if ON_TILE(AUDIO_PIPELINE_TILE_NO)

    asr_sample_t asr_buf[appconfAUDIO_PIPELINE_FRAME_ADVANCE] = {0};

    for (int i = 0; i < frame_count; i++) {
        asr_buf[i] = ((int32_t *)output_audio_frames)[i] >> 16;
    }
    vPortFree(output_audio_frames);

    wakeword_result_t ww_res = wakeword_handler((asr_sample_t *)asr_buf, frame_count);

    switch (ww_res) {
        case WAKEWORD_ERROR:
            power_control_halt();
            power_state_set(POWER_STATE_FULL);
            break;
        case WAKEWORD_FOUND:
            power_state_set(POWER_STATE_FULL);
        default:
        case WAKEWORD_NOT_FOUND:
            break;
    }

#if LOW_POWER_AUDIO_BUFFER_ENABLED
    const uint32_t max_dequeue_packets = 1;
    const uint32_t max_dequeued_samples = (max_dequeue_packets * appconfAUDIO_PIPELINE_FRAME_ADVANCE);

    if (power_control_state_get() == POWER_STATE_FULL) {
        if (low_power_audio_buffer_dequeue(max_dequeue_packets) == max_dequeued_samples) {
            // Max data has been dequeued, enqueue the newest data.
            low_power_audio_buffer_enqueue(asr_buf, frame_count);
        } else {
            // More data can be sent.
            intent_engine_sample_push(asr_buf, frame_count);
        }
    } else {
        low_power_audio_buffer_enqueue(asr_buf, frame_count);
    }
#else // LOW_POWER_AUDIO_BUFFER_ENABLED
    if (power_control_state_get() == POWER_STATE_FULL) {
        intent_engine_sample_push(asr_buf, frame_count);
    }
#endif // LOW_POWER_AUDIO_BUFFER_ENABLED
#endif // ON_TILE(AUDIO_PIPELINE_TILE_NO)

    return AUDIO_PIPELINE_DONT_FREE_FRAME;
}

void vApplicationMallocFailedHook(void)
{
#if (configUSE_TRACE_FACILITY == 1)
    TaskStatus_t status;
    vTaskGetInfo(NULL, &status, pdTRUE, eInvalid );
    rtos_printf("Malloc Failed on tile %d in %s!\n", THIS_XCORE_TILE, status.pcTaskName);
#else
    rtos_printf("Malloc Failed on tile %d!\n", THIS_XCORE_TILE);
#endif
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

#if ON_TILE(MICARRAY_TILE_NO)
    no_preempt_task_create();
#endif

#if ON_TILE(0)
    // Setup flash low-level mode
    //   NOTE: must call rtos_qspi_flash_fast_read_shutdown_ll to use non low-level mode calls
    rtos_qspi_flash_fast_read_setup_ll(qspi_flash_ctx);
    led_task_create(appconfLED_TASK_PRIORITY, NULL);
#endif

#if ON_TILE(1)
    gpio_gpi_init(gpio_ctx_t0);
#endif

#if ON_TILE(FS_TILE_NO)
    rtos_fatfs_init(qspi_flash_ctx);
#endif

    power_control_task_create(appconfPOWER_CONTROL_TASK_PRIORITY, NULL);

#if ON_TILE(WAKEWORD_TILE_NO)
    power_state_init();
    wakeword_init();
#endif

#if ON_TILE(ASR_TILE_NO)
    QueueHandle_t q_intent = xQueueCreate(appconfINTENT_QUEUE_LEN, sizeof(int32_t));
    intent_handler_create(appconfINTENT_MODEL_RUNNER_TASK_PRIORITY, q_intent);
    intent_engine_create(appconfINTENT_MODEL_RUNNER_TASK_PRIORITY, q_intent);
#endif

#if ON_TILE(AUDIO_PIPELINE_TILE_NO)
    // Wait until the intent engine is initialized before starting the
    // audio pipeline.
    intent_engine_ready_sync();
    audio_pipeline_init(NULL, NULL);

    set_local_tile_processor_clk_div(1);
    enable_local_tile_processor_clock_divider();
    set_local_tile_processor_clk_div(appconfLOW_POWER_CONTROL_TILE_CLK_DIV);
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

void tile_common_init(chanend_t c)
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
