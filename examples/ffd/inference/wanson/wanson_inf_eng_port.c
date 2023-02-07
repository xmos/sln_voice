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
#include "power/power_state.h"

static QueueHandle_t q_intent = 0;
static uint8_t keyword_proc_busy = 0;

// look up table to converting ASR IDs to wav file IDs or strings
typedef struct asr_lut_struct
{
    int     asr_id;    // ASR response IDs
    int     wav_id;    // Wav file IDs corresponding to audio_files_en[] array in audio_response.c
    const char* text;  // String output
} asr_lut_t;

static asr_lut_t asr_keyword_lut[ASR_NUMBER_OF_KEYWORDS] = {
    {ASR_KEYWORD_HELLO_XMOS, 1, "Hello XMOS"},
    {ASR_KEYWORD_ALEXA, 1, "Alexa"},
};
static asr_lut_t asr_command_lut[ASR_NUMBER_OF_COMMANDS] = {
    {ASR_COMMAND_TV_ON, 2, "Switch on the TV"},
    {ASR_COMMAND_TV_OFF, 3, "Switch off the TV"},
    {ASR_COMMAND_VOLUME_UP, 6, "Volume up"},
    {ASR_COMMAND_VOLUME_DOWN, 7, "Volume down"},
    {ASR_COMMAND_CHANNEL_UP, 4, "Channel up"},
    {ASR_COMMAND_CHANNEL_DOWN, 5, "Channel down"},
    {ASR_COMMAND_LIGHTS_ON, 8, "Switch on the lights"},
    {ASR_COMMAND_LIGHTS_OFF, 9, "Switch off the lights"},
    {ASR_COMMAND_LIGHTS_UP, 10, "Brightness up"},
    {ASR_COMMAND_LIGHTS_DOWN, 11, "Brightness down"},
    {ASR_COMMAND_FAN_ON, 12, "Switch on the fan"},
    {ASR_COMMAND_FAN_OFF, 13, "Switch off the fan"},
    {ASR_COMMAND_FAN_UP, 14, "Speed up the fan"},
    {ASR_COMMAND_FAN_DOWN, 15, "Slow down the fan"},
    {ASR_COMMAND_TEMPERATURE_UP, 16, "Set higher temperature"},
    {ASR_COMMAND_TEMPERATURE_DOWN, 17, "Set lower temperature"}
};

void wanson_engine_play_response(int wav_id)
{
    if(q_intent != 0) {
        keyword_proc_busy = 1;
        if(xQueueSend(q_intent, (void *)&wav_id, (TickType_t)0) != pdPASS) {
            rtos_printf("Lost wav playback.  Queue was full.\n");
            keyword_proc_busy = 0;
        }
    }
}

void wanson_engine_process_asr_result(asr_keyword_t keyword, asr_command_t command)
{
    int wav_id = 0;
    const char* text = "";
    if (keyword != ASR_KEYWORD_UNKNOWN) {
        for (int i=0; i<ASR_NUMBER_OF_KEYWORDS; i++) {
            if (asr_keyword_lut[i].asr_id == keyword) {
                wav_id = asr_keyword_lut[i].wav_id;
                text = asr_keyword_lut[i].text;
            }
        }
        rtos_printf("KEYWORD: 0x%x, %s\n", (int) keyword, (char*)text);
        wanson_engine_play_response(wav_id);
    } else if (command != ASR_COMMAND_UNKNOWN) {
        for (int i=0; i<ASR_NUMBER_OF_COMMANDS; i++) {
            if (asr_command_lut[i].asr_id == command) {
                wav_id = asr_command_lut[i].wav_id;
                text = asr_command_lut[i].text;
            }
        }
        rtos_printf("KEYWORD: 0x%x, %s\n", (int) command, (char*)text);
        wanson_engine_play_response(wav_id);
    }
}

#if appconfLOW_POWER_ENABLED && ON_TILE(INFERENCE_TILE_NO)
void inference_engine_full_power_request(void)
{
    wanson_engine_full_power_request();
}

void inference_engine_low_power_accept(void)
{
    wanson_engine_low_power_accept();
}

uint8_t inference_engine_low_power_ready(void)
{
    return (keyword_proc_busy == 0);
}

void inference_engine_low_power_reset(void)
{
    wanson_engine_stream_buf_reset();
    xQueueReset(q_intent);
}

int32_t inference_engine_keyword_queue_count(void)
{
    return (q_intent != NULL) ? (int32_t)uxQueueMessagesWaiting(q_intent) : 0;
}

void inference_engine_keyword_queue_complete(void)
{
    keyword_proc_busy = 0;
}
#endif /* appconfLOW_POWER_ENABLED && ON_TILE(INFERENCE_TILE_NO) */

#if appconfINFERENCE_ENABLED && ON_TILE(INFERENCE_TILE_NO)
int32_t inference_engine_create(uint32_t priority, void *args)
{
    q_intent = (QueueHandle_t) args;

#if INFERENCE_TILE_NO == AUDIO_PIPELINE_TILE_NO
    wanson_engine_task_create(priority);
#else
    wanson_engine_intertile_task_create(priority);
#endif
    return 0;
}
#endif /* appconfINFERENCE_ENABLED && ON_TILE(INFERENCE_TILE_NO) */

int32_t inference_engine_sample_push(int32_t *buf, size_t frames)
{
#if appconfINFERENCE_ENABLED && ON_TILE(AUDIO_PIPELINE_TILE_NO)
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
