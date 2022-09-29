// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

/* STD headers */
#include <platform.h>
#include <xs1.h>
#include <xcore/hwtimer.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"
#include "queue.h"

/* App headers */
#include "app_conf.h"
#include "platform/driver_instances.h"
#include "inference_engine.h"
#include "wanson_inf_eng.h"
#include "ssd1306_rtos_support.h"

static QueueHandle_t q_intent = 0;

__attribute__((weak))
void wanson_engine_proc_keyword_result(const char **text, int id)
{
    if(text != NULL) {
        rtos_printf("KEYWORD: 0x%x, %s\n", id, (char*)*text);
    }
    if(q_intent != 0) {
        if(xQueueSend(q_intent, (void *)&id, (TickType_t)0) != pdPASS) {
            rtos_printf("Lost intent.  Queue was full.\n");
        }
    }

#if appconfSSD1306_DISPLAY_ENABLED
    // some temporary fixes to the strings returned
    switch (id) {
        case 50:
            // Clear the display
            ssd1306_display_ascii_to_bitmap("\0");
            break;
        case 3:
            // fix capital "On"
            ssd1306_display_ascii_to_bitmap("Switch on the TV\0");
            break;
        case 15:
            // fix lower case "speed"
            // fix word wrapping
            ssd1306_display_ascii_to_bitmap("Speed up the   fan\0");
            break;
        case 16:
            // fix lower case "slow"
            ssd1306_display_ascii_to_bitmap("Slow down the fan\0");
            break;
        case 17:
            // fix lower case "set"
            // fix word wrapping
            ssd1306_display_ascii_to_bitmap("Set higher    temperature\0");
            break;
        case 18:
            // fix lower case "set"
            // fix word wrapping
            ssd1306_display_ascii_to_bitmap("Set lower     temperature\0");
            break;
        default:
            if(text != NULL) {
                ssd1306_display_ascii_to_bitmap((char *)*text);
            }
    }
#endif
}

int32_t inference_engine_create(uint32_t priority, void *args)
{
    q_intent = (QueueHandle_t) args;

#if appconfINFERENCE_ENABLED
#if INFERENCE_TILE_NO == AUDIO_PIPELINE_TILE_NO
    wanson_engine_task_create(priority);
#else
    wanson_engine_intertile_task_create(priority);
#endif
#endif
    return 0;
}

int32_t inference_engine_sample_push(int32_t *buf, size_t frames)
{
#if appconfINFERENCE_ENABLED
#if INFERENCE_TILE_NO == AUDIO_PIPELINE_TILE_NO
    wanson_engine_samples_send_local(
            frames,
            buf);
#else
    wanson_engine_samples_send_remote(
            intertile_ctx,
            frames,
            buf);
#endif
#endif
    return 0;
}
