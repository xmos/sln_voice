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
#include "intent_engine.h"

static QueueHandle_t q_intent = 0;
static uint8_t keyword_proc_busy = 0;

// look up table to converting ASR IDs to wav file IDs or strings
#define ASR_NUMBER_OF_COMMANDS  (17)

typedef struct asr_lut_struct
{
    int     asr_id;    // ASR response IDs
    int     wav_id;    // Wav file IDs corresponding to audio_files_en[] array in audio_response.c
    const char* text;  // String output
} asr_lut_t;

static asr_lut_t asr_lut[ASR_NUMBER_OF_COMMANDS] = {
    {1, 1, "Hello XMOS"},
    {3, 2, "Switch on the TV"},
    {4, 3, "Switch off the TV"},
    {5, 4, "Channel up"},
    {6, 5, "Channel down"},
    {7, 6, "Volume up"},
    {8, 7, "Volume down"},
    {9, 8, "Switch on the lights"},
    {10, 9, "Switch off the lights"},
    {11, 10, "Brightness up"},
    {12, 11, "Brightness down"},
    {13, 12, "Switch on the fan"},
    {14, 13, "Switch off the fan"},
    {15, 14, "Speed up the fan"},
    {16, 15, "Slow down the fan"},
    {17, 16, "Set higher temperature"},
    {18, 17, "Set lower temperature"}
};

void intent_engine_play_response(int wav_id)
{
    if(q_intent != 0) {
        keyword_proc_busy = 1;
        if(xQueueSend(q_intent, (void *)&wav_id, (TickType_t)0) != pdPASS) {
            rtos_printf("Lost wav playback.  Queue was full.\n");
            keyword_proc_busy = 0;
        }
    }
}

void intent_engine_process_asr_result(int word_id)
{
    int wav_id = 0;
    const char* text = "";

    for (int i=0; i<ASR_NUMBER_OF_COMMANDS; i++) {
        if (asr_lut[i].asr_id == word_id) {
            wav_id = asr_lut[i].wav_id;
            text = asr_lut[i].text;
        }
    }
    rtos_printf("KEYWORD: 0x%x, %s\n", (int) word_id, (char*)text);
    intent_engine_play_response(wav_id);
}

#if appconfLOW_POWER_ENABLED && ON_TILE(ASR_TILE_NO)

uint8_t intent_engine_low_power_ready(void)
{
    return (keyword_proc_busy == 0);
}

void intent_engine_low_power_reset(void)
{
    intent_engine_stream_buf_reset();
    xQueueReset(q_intent);
}

int32_t intent_engine_keyword_queue_count(void)
{
    return (q_intent != NULL) ? (int32_t)uxQueueMessagesWaiting(q_intent) : 0;
}

void intent_engine_keyword_queue_complete(void)
{
    keyword_proc_busy = 0;
}
#endif /* appconfLOW_POWER_ENABLED && ON_TILE(ASR_TILE_NO) */

#if appconfINTENT_ENABLED && ON_TILE(ASR_TILE_NO)
int32_t intent_engine_create(uint32_t priority, void *args)
{
    q_intent = (QueueHandle_t) args;

#if ASR_TILE_NO == AUDIO_PIPELINE_TILE_NO
    intent_engine_task_create(priority);
#else
    intent_engine_intertile_task_create(priority);
#endif
    return 0;
}
#endif /* appconfINTENT_ENABLED && ON_TILE(ASR_TILE_NO) */

int32_t intent_engine_sample_push(int32_t *buf, size_t frames)
{
#if appconfINTENT_ENABLED && ON_TILE(AUDIO_PIPELINE_TILE_NO)
#if ASR_TILE_NO == AUDIO_PIPELINE_TILE_NO
    intent_engine_samples_send_local(
            frames,
            buf);
#else
    intent_engine_samples_send_remote(
            intertile_ap_ctx,
            frames,
            buf);
#endif
#endif
    return 0;
}
