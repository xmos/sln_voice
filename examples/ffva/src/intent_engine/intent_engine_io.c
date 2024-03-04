// Copyright 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

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

// look up table to converting ASR IDs to wav file IDs or strings
#define ASR_NUMBER_OF_COMMANDS  (17)

typedef struct asr_lut_struct
{
    int     asr_id;    // ASR response IDs
    int     wav_id;    // Wav file IDs corresponding to audio_files_en[] array in audio_response.c
    const char* text;  // String output
} asr_lut_t;

#if ASR_CYBERON
static asr_lut_t asr_lut[ASR_NUMBER_OF_COMMANDS] = {
    {1, 1, "Hello XMOS"},
    {2, 2, "Switch on the TV"},
    {3, 3, "Switch off the TV"},
    {4, 4, "Channel up"},
    {5, 5, "Channel down"},
    {6, 6, "Volume up"},
    {7, 7, "Volume down"},
    {8, 8, "Switch on the lights"},
    {9, 9, "Switch off the lights"},
    {10, 10, "Brightness up"},
    {11, 11, "Brightness down"},
    {12, 12, "Switch on the fan"},
    {13, 13, "Switch off the fan"},
    {14, 14, "Speed up the fan"},
    {15, 15, "Slow down the fan"},
    {16, 16, "Set higher temperature"},
    {17, 17, "Set lower temperature"}
};
#elif ASR_SENSORY
static asr_lut_t asr_lut[ASR_NUMBER_OF_COMMANDS] = {
    {1, 2, "Switch on the TV"},
    {2, 4, "Channel up"},
    {3, 5, "Channel down"},
    {4, 6, "Volume up"},
    {5, 7, "Volume down"},
    {6, 3, "Switch off the TV"},
    {7, 8, "Switch on the lights"},
    {8, 10, "Brightness up"},
    {9, 11, "Brightness down"},
    {10, 9, "Switch off the lights"},
    {11, 12, "Switch on the fan"},
    {12, 14, "Speed up the fan"},
    {13, 15, "Slow down the fan"},
    {14, 16, "Set higher temperature"},
    {15, 17, "Set lower temperature"},
    {16, 13, "Switch off the fan"},
    {17, 1, "Hello XMOS"}
};
#else
#error "Model has to be either Sensory or Cyberon"
#endif



void intent_engine_play_response(int wav_id)
{
    if(q_intent != 0) {
        if(xQueueSend(q_intent, (void *)&wav_id, (TickType_t)0) != pdPASS) {
            rtos_printf("Lost wav playback.  Queue was full.\n");
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
    rtos_printf("RECOGNIZED: 0x%x, %s\n", (int) word_id, (char*)text);
    intent_engine_play_response(wav_id);
}

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
