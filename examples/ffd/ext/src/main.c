// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

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

/* App headers */
#include "app_conf.h"
#include "platform/platform_init.h"
#include "platform/driver_instances.h"
#include "usb_support.h"
#include "usb_audio.h"
#include "audio_pipeline.h"
#include "intent_engine.h"
#include "fs_support.h"
#include "gpio_ctrl/gpi_ctrl.h"
#include "rtos_swmem.h"
#include "ssd1306_rtos_support.h"

volatile int mic_from_usb = appconfMIC_SRC_DEFAULT;

void audio_pipeline_input(void *input_app_data,
                          int32_t **input_audio_frames,
                          size_t ch_count,
                          size_t frame_count)
{
    (void) input_app_data;

#if appconfUSB_ENABLED && appconfUSB_AUDIO_ENABLED
    int32_t **usb_mic_audio_frame = NULL;

    if (mic_from_usb) {
        usb_mic_audio_frame = input_audio_frames;
        usb_audio_recv(intertile_usb_ctx,
                       frame_count,
                       usb_mic_audio_frame,
                       ch_count);
    } else {
        rtos_mic_array_rx(mic_array_ctx,
                          input_audio_frames,
                          frame_count,
                          portMAX_DELAY);
    }
#else
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
#endif
}

int audio_pipeline_output(void *output_app_data,
                          int32_t **output_audio_frames,
                          size_t ch_count,
                          size_t frame_count)
{
    (void) output_app_data;

#if appconfI2S_ENABLED
    /* I2S expects sample channel format */
    int32_t tmp[appconfAUDIO_PIPELINE_FRAME_ADVANCE][appconfAUDIO_PIPELINE_CHANNELS];
    for (int j=0; j<appconfAUDIO_PIPELINE_FRAME_ADVANCE; j++) {
        /* ASR output is first */
        tmp[j][0] = (int32_t)output_audio_frames[j];
        tmp[j][1] = (int32_t)output_audio_frames[j];
    }

    rtos_i2s_tx(i2s_ctx,
                (int32_t*) tmp,
                frame_count,
                portMAX_DELAY);
#endif

#if appconfUSB_ENABLED && appconfUSB_AUDIO_ENABLED
    usb_audio_send(intertile_usb_ctx,
                frame_count,
                output_audio_frames,
                4);
#endif

#if appconfINTENT_ENABLED
    intent_engine_sample_push((int32_t *)output_audio_frames, frame_count);
#endif

    return AUDIO_PIPELINE_FREE_FRAME;
}

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

#if appconfINTENT_ENABLED && ON_TILE(ASR_TILE_NO)
#if SSD1306_DISPLAY_ENABLED
    ssd1306_display_create((configMAX_PRIORITIES / 2 - 1));
#endif
    intent_engine_create(appconfINTENT_MODEL_RUNNER_TASK_PRIORITY, NULL);
#endif

#if ON_TILE(AUDIO_PIPELINE_TILE_NO)
    audio_pipeline_init(NULL, NULL);

#if appconfINTENT_ENABLED 
    // In the normal application we wait until the 
    // inference engine is running before starting the audio pipeline.
    // Due to adaptive usb timing constraints we cannot
    // wait in the FFD audio test build, so we initialize the pipeline
    // always, and then consume the Wanson ready packet.
    // This results in warning prints about samples being lost to
    // the inference engine during the period where the engine is
    // being set up.
    {
        int ret = 0;
        rtos_intertile_rx_len(intertile_ctx, appconfINTENT_ENGINE_READY_SYNC_PORT, RTOS_OSAL_WAIT_FOREVER);
        rtos_intertile_rx_data(intertile_ctx, &ret, sizeof(ret));
    }
#endif
#endif

	for (;;) {
		// rtos_printf("Tile[%d]:\n\tMinimum heap free: %d\n\tCurrent heap free: %d\n", THIS_XCORE_TILE, xPortGetMinimumEverFreeHeapSize(), xPortGetFreeHeapSize());
		vTaskDelay(pdMS_TO_TICKS(5000));
	}
}

void tile_common_init(chanend_t c)
{
    platform_init(c);
    chanend_free(c);

#if appconfUSB_ENABLED && appconfUSB_AUDIO_ENABLED && ON_TILE(USB_TILE_NO)
    usb_audio_init(intertile_usb_ctx, appconfUSB_AUDIO_TASK_PRIORITY);
#endif

    xTaskCreate((TaskFunction_t) startup_task,
                "startup_task",
                RTOS_THREAD_STACK_SIZE(startup_task),
                NULL,
                appconfSTARTUP_TASK_PRIORITY,
                NULL);

    rtos_printf("start scheduler on tile %d\n", THIS_XCORE_TILE);
    vTaskStartScheduler();
}
