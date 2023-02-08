// Copyright (c) 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public License: Version 1

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
#include "audio_pipeline/audio_pipeline.h"
#include "inference_engine.h"
#include "fs_support.h"
#include "gpio_ctrl/gpi_ctrl.h"
#include "gpio_ctrl/leds.h"
#include "intent_handler/intent_handler.h"
#include "power/lp_control.h"
#include "xmath/xmath.h"


extern void startup_task(void *arg);
extern void tile_common_init(chanend_t c);

#if appconfLOW_POWER_ENABLED
rtos_low_power_t lp_ctx_s;
rtos_low_power_t *lp_ctx = &lp_ctx_s;
#endif

#if LOW_POWER_AUDIO_BUFFER_ENABLED
StreamBufferHandle_t audio_buffer = NULL;
#endif

__attribute__((weak))
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

__attribute__((weak))
int audio_pipeline_output(void *output_app_data,
                          int32_t **output_audio_frames,
                          size_t ch_count,
                          size_t frame_count)
{
    
#if ON_TILE(AUDIO_PIPELINE_TILE_NO) && !appconfLOW_POWER_ENABLED
    if (audio_buffer == NULL) {
        audio_buffer = xStreamBufferCreate((size_t)(appconfAUDIO_PIPELINE_BUFFER_NUM_PACKETS * appconfAUDIO_PIPELINE_FRAME_ADVANCE * sizeof(int32_t)), 0);
    }
        size_t bytes_sent = xStreamBufferSend(
                                audio_buffer,
                                (int32_t *)output_audio_frames,
                                sizeof(int32_t) * frame_count,
                                0);

    configASSERT(bytes_sent == (sizeof(int32_t) * frame_count));
            size_t bytes_rxed = xStreamBufferReceive(
                                    audio_buffer,
                                    (int32_t *)output_audio_frames,
                                    sizeof(int32_t) * frame_count,
                                    portMAX_DELAY);
                                    
        configASSERT(bytes_rxed == (sizeof(int32_t) * frame_count));
#if appconfINFERENCE_ENABLED
        inference_engine_sample_push((int32_t *)output_audio_frames, frame_count);
#endif /* appconfINFERENCE_ENABLED */
#endif /* ON_TILE(AUDIO_PIPELINE_TILE_NO) && !appconfLOW_POWER_ENABLED */

#if ON_TILE(AUDIO_PIPELINE_TILE_NO) && appconfLOW_POWER_ENABLED
    /* Extract the vnr and ema data from the frame */
    uint8_t *tmp_ptr = (uint8_t *)output_audio_frames;
    uint32_t vnr_pred_offset = 2 * appconfAUDIO_PIPELINE_CHANNELS * appconfAUDIO_PIPELINE_FRAME_ADVANCE * sizeof(int32_t);
    uint32_t ema_energy_offset = vnr_pred_offset + sizeof(float_s32_t);
    float_s32_t vnr_pred_tmp;
    float_s32_t ema_energy_tmp;
    memcpy(&vnr_pred_tmp, tmp_ptr + vnr_pred_offset, sizeof(float_s32_t));
    memcpy(&ema_energy_tmp, tmp_ptr + ema_energy_offset, sizeof(float_s32_t));

    float vnr_pred = float_s32_to_float(vnr_pred_tmp);
    float ema_energy = float_s32_to_float(ema_energy_tmp);

    if ((ema_energy >= appconfPOWER_HIGH_ENERGY_THRESHOLD) ||
        ((ema_energy >= appconfPOWER_LOW_ENERGY_THRESHOLD) &&
         (vnr_pred >= appconfPOWER_VNR_THRESHOLD))) {
            lp_master_voice_activity_present(lp_ctx);
    }

    power_state_t power_state = get_lp_power_state_ll(lp_ctx);

#if LOW_POWER_AUDIO_BUFFER_ENABLED
    size_t bytes_sent = xStreamBufferSend(
                                audio_buffer,
                                (int32_t *)output_audio_frames,
                                sizeof(int32_t) * frame_count,
                                0);

    configASSERT(bytes_sent == (sizeof(int32_t) * frame_count));

    if (power_state != POWER_STATE_LOW) {
        size_t bytes_rxed = xStreamBufferReceive(
                                    audio_buffer,
                                    (int32_t *)output_audio_frames,
                                    sizeof(int32_t) * frame_count,
                                    portMAX_DELAY);
                                    
        configASSERT(bytes_rxed == (sizeof(int32_t) * frame_count));
#if appconfINFERENCE_ENABLED
        inference_engine_sample_push((int32_t *)output_audio_frames, frame_count);
#endif /* appconfINFERENCE_ENABLED */
    } else {
        if (xStreamBufferSpacesAvailable(audio_buffer) < (sizeof(int32_t) * frame_count)) {
            /* drop oldest data */
            size_t bytes_rxed = xStreamBufferReceive(
                                        audio_buffer,
                                        (int32_t *)output_audio_frames,
                                        sizeof(int32_t) * frame_count,
                                        portMAX_DELAY);
                                        
            configASSERT(bytes_rxed == (sizeof(int32_t) * frame_count));
        }
    }
#else
    if (power_state != POWER_STATE_LOW) {
#if appconfINFERENCE_ENABLED
        inference_engine_sample_push((int32_t *)output_audio_frames, frame_count);
#endif /* appconfINFERENCE_ENABLED */
    }
#endif /* LOW_POWER_AUDIO_BUFFER_ENABLED */
#endif /* ON_TILE(AUDIO_PIPELINE_TILE_NO) && appconfLOW_POWER_ENABLED */

    return AUDIO_PIPELINE_FREE_FRAME;
}

void vApplicationMallocFailedHook(void)
{
    rtos_printf("Malloc Failed on tile %d!\n", THIS_XCORE_TILE);
    xassert(0);
    for(;;);
}

static void mem_analysis(void)
{
	for (;;) {
        ;
		// rtos_printf("Tile[%d]:\n\tMinimum heap free: %d\n\tCurrent heap free: %d\n", THIS_XCORE_TILE, xPortGetMinimumEverFreeHeapSize(), xPortGetFreeHeapSize());
		// vTaskDelay(pdMS_TO_TICKS(5000));
	}
}

__attribute__((weak))
void startup_task(void *arg)
{
    rtos_printf("Startup task running from tile %d on core %d\n", THIS_XCORE_TILE, portGET_CORE_ID());

    platform_start();

#if ON_TILE(1)
    gpio_gpi_init(gpio_ctx_t0);
#endif

#if ON_TILE(FS_TILE_NO)
    rtos_fatfs_init(qspi_flash_ctx);
#endif

#if appconfINFERENCE_ENABLED && ON_TILE(INFERENCE_TILE_NO)
    QueueHandle_t q_intent = xQueueCreate(appconfINTENT_QUEUE_LEN, sizeof(int32_t));
    intent_handler_create(appconfINFERENCE_MODEL_RUNNER_TASK_PRIORITY, q_intent);
    inference_engine_create(appconfINFERENCE_MODEL_RUNNER_TASK_PRIORITY, q_intent);
#endif

#if ON_TILE(0)
    led_task_create(appconfLED_TASK_PRIORITY, NULL);
#endif

#if ON_TILE(AUDIO_PIPELINE_TILE_NO)
#if appconfINFERENCE_ENABLED
    // Wait until the Wanson engine is initialized before we start the
    // audio pipeline.
    {
        int ret = 0;
        rtos_intertile_rx_len(intertile_ctx, appconfWANSON_READY_SYNC_PORT, RTOS_OSAL_WAIT_FOREVER);
        rtos_intertile_rx_data(intertile_ctx, &ret, sizeof(ret));
    }
#endif
#if LOW_POWER_AUDIO_BUFFER_ENABLED
    audio_buffer = xStreamBufferCreate((size_t)(appconfAUDIO_PIPELINE_BUFFER_NUM_PACKETS * appconfAUDIO_PIPELINE_FRAME_ADVANCE * sizeof(int32_t)), 0);
#endif
    audio_pipeline_init(NULL, NULL);
#endif

#if appconfLOW_POWER_ENABLED
#if ON_TILE(0)
    lp_slave_task_create(lp_ctx, appconfPOWER_CONTROL_TASK_PRIORITY, NULL);
#endif
#if ON_TILE(1)
    lp_master_task_create(lp_ctx, appconfPOWER_CONTROL_TASK_PRIORITY, NULL);
    vTaskDelay(pdMS_TO_TICKS(10));
    set_local_tile_processor_clk_div(1);
    enable_local_tile_processor_clock_divider();
    set_local_tile_processor_clk_div(appconfLOW_POWER_CONTROL_TILE_CLK_DIV);
#endif
#endif

    // mem_analysis();
    vTaskSuspend(NULL);
    while(1){;} /* Trap */
}

void vApplicationMinimalIdleHook(void)
{
    rtos_printf("idle hook on tile %d core %d\n", THIS_XCORE_TILE, rtos_core_id_get());
    asm volatile("waiteu");
}

__attribute__((weak))
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
