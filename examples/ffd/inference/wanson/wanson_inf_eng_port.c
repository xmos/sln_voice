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


// This enum table should match audio_files_en[] array in audio_response.c
enum WAV_FILES {
    SLEEP_WAV,
    WAKEUP_WAV,
    TVON_WAV,
    TVOFF_WAV,
    CHUP_WAV,
    CHDOWN_WAV,
    VOLUP_WAV,
    VOLDOWN_WAV,
    LIGHTON_WAV,
    LIGHTOFF_WAV,
    LIGHTSUP_WAV,
    LIGHTSDOWN_WAV,
    FANON_WAV,
    FANOFF_WAV,
    FANUP_WAV,
    FANDOWN_WAV,
    TEMPUP_WAV,
    TEMPDOWN_WAV,
    TOTAL_WAV_NUM
};

// wanson_id_intent_conv: look up table to converting Wanson's audio response IDs to intent IDs.
// 1st column: Wanson's audio repsonse IDs, 2nd column: Intent IDs corresponding to audio_files_en[] array in audio_response.c

static int wanson_id_intent_id[TOTAL_WAV_NUM][2] = {
    {50,SLEEP_WAV},
    {1,WAKEUP_WAV},
    {2,WAKEUP_WAV},
    {3,TVON_WAV},
    {4,TVOFF_WAV},
    {5,CHUP_WAV},
    {6,CHDOWN_WAV},
    {7,VOLUP_WAV},
    {8,VOLDOWN_WAV},
    {9,LIGHTON_WAV},
    {10,LIGHTOFF_WAV},
    {11,LIGHTSUP_WAV},
    {12,LIGHTSDOWN_WAV},
    {13,FANON_WAV},
    {14,FANOFF_WAV},
    {15,FANUP_WAV},
    {16,FANDOWN_WAV},
    {17,TEMPUP_WAV},
    {18,TEMPDOWN_WAV}
};


int wanson_id_intent_id_conv(int wan_id)
{
    uint16_t i;
    
    for(i=0;i<sizeof(wanson_id_intent_id)/2;i++){
        if(wanson_id_intent_id[i][0] == wan_id){
            return wanson_id_intent_id[i][1];
        }
    }
    return 0xFF;
}

__attribute__((weak))
void wanson_engine_proc_keyword_result(const char **text, int id)
{
    if(text != NULL) {
        rtos_printf("KEYWORD: 0x%x, %s\n", id, (char*)*text);
    }
    if(q_intent != 0) {
        int wav_id = 0;
        wav_id = wanson_id_intent_id_conv(id);
        if(xQueueSend(q_intent, (void *)&wav_id, (TickType_t)0) != pdPASS) {
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
