// Copyright (c) 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public License: Version 1

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

#if ON_TILE(ASR_TILE_NO)

static QueueHandle_t q_intent = 0;
static uint8_t keyword_proc_busy = 0;

// look up table to converting ASR IDs to wav file IDs or strings
#define ASR_NUMBER_OF_COMMANDS  (16)

typedef struct asr_lut_struct
{
    int     asr_id;    // ASR response IDs
    const char* text;  // String output
} asr_lut_t;

static asr_lut_t asr_lut[ASR_NUMBER_OF_COMMANDS] = {
    {1, "Switch on the TV"},
    {2, "Channel up"},
    {3, "Channel down"},
    {4, "Volume up"},
    {5, "Volume down"},
    {6, "Switch off the TV"},
    {7, "Switch on the lights"},
    {8, "Brightness up"},
    {9, "Brightness down"},
    {10, "Switch off the lights"},
    {11, "Switch on the fan"},
    {12, "Speed up the fan"},
    {13, "Slow down the fan"},
    {14, "Set higher temperature"},
    {15, "Set lower temperature"},
    {16, "Switch off the fan"}
};

void intent_engine_process_asr_result(int word_id)
{
    const char* text = "UNKNOWN";

    for (int i = 0; i < ASR_NUMBER_OF_COMMANDS; i++) {
        if (asr_lut[i].asr_id == word_id) {
            text = asr_lut[i].text;
            break;
        }
    }
    rtos_printf("KEYWORD: 0x%x, %s\n", (int) word_id, (char*)text);
    if (q_intent != 0) {
        keyword_proc_busy = 1;
        if (xQueueSend(q_intent, (void *)&word_id, (TickType_t)0) != pdPASS) {
            rtos_printf("Lost ASR result.  Queue was full.\n");
            keyword_proc_busy = 0;
        }
    }
}

uint8_t intent_engine_low_power_ready(void)
{
    return (keyword_proc_busy == 0);
}

void intent_engine_keyword_queue_reset(void)
{
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

#endif /* ON_TILE(ASR_TILE_NO) */

#if appconfINTENT_ENABLED && ON_TILE(ASR_TILE_NO)
int32_t intent_engine_create(uint32_t priority, void *args)
{
    q_intent = (QueueHandle_t) args;
    intent_engine_intertile_task_create(priority);
    return 0;
}
#endif /* appconfINTENT_ENABLED && ON_TILE(ASR_TILE_NO) */

int32_t intent_engine_sample_push(int32_t *buf, size_t frames)
{
#if appconfINTENT_ENABLED && ON_TILE(AUDIO_PIPELINE_TILE_NO)
    intent_engine_samples_send_remote(
            intertile_ap_ctx,
            frames,
            buf);
#endif
    return 0;
}
