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

/* App headers */
#include "app_conf.h"
#include "platform/driver_instances.h"
#include "inference_engine.h"
#include "wanson_inf_eng.h"
#include "asr.h"
#include "gpio_ctrl/leds.h"
#if appconfLOW_POWER_ENABLED
#include "power/lp_control.h"
#endif

#if ON_TILE(INFERENCE_TILE_NO)

#define IS_KEYWORD(id)    (id != ASR_KEYWORD_UNKNOWN)
#define IS_COMMAND(id)    (id != ASR_COMMAND_UNKNOWN)

#define WANSON_SAMPLES_PER_INFERENCE    (2 * appconfINFERENCE_SAMPLE_BLOCK_LENGTH)

#define STOP_LISTENING_SOUND_WAV_ID     (0)

typedef enum inference_state {
    STATE_EXPECTING_WAKEWORD,
    STATE_EXPECTING_COMMAND,
    STATE_PROCESSING_COMMAND
} inference_state_t;

static inference_state_t inference_state;
static asr_context_t asr_ctx; 

static void vInferenceTimerCallback(TimerHandle_t pxTimer);
static void receive_audio_frames(StreamBufferHandle_t input_queue, int32_t *buf,
                                 int16_t *buf_short, size_t *buf_short_index);

static void vInferenceTimerCallback(TimerHandle_t pxTimer)
{
    if ((inference_state == STATE_EXPECTING_COMMAND)
        || (inference_state == STATE_PROCESSING_COMMAND)) {
        wanson_engine_play_response(STOP_LISTENING_SOUND_WAV_ID);
        led_indicate_waiting();
        inference_state = STATE_EXPECTING_WAKEWORD;
    }
    xTimerStop(pxTimer, 0);
#if appconfLOW_POWER_ENABLED
    lp_slave_user_not_active(lp_ctx, LP_SLAVE_LP_INT_TIMEOUT_HANDLER);
#endif
}

static void receive_audio_frames(StreamBufferHandle_t input_queue, int32_t *buf,
                                 int16_t *buf_short, size_t *buf_short_index)
{
    uint8_t *buf_ptr = (uint8_t*)buf;
    size_t buf_len = appconfINFERENCE_SAMPLE_BLOCK_LENGTH * sizeof(int32_t);

    do {
        size_t bytes_rxed = xStreamBufferReceive(input_queue,
                                                 buf_ptr,
                                                 buf_len,
                                                 portMAX_DELAY);
        buf_len -= bytes_rxed;
        buf_ptr += bytes_rxed;
    } while (buf_len > 0);

    for (int i = 0; i < appconfINFERENCE_SAMPLE_BLOCK_LENGTH; i++) {
        buf_short[(*buf_short_index)++] = buf[i] >> 16;
    }
}

static void hold_inf_state(TimerHandle_t pxTimer)
{
#if appconfLOW_POWER_ENABLED
    lp_slave_user_active(lp_ctx, LP_SLAVE_LP_INT_TIMEOUT_HANDLER);
#endif
    xTimerReset(pxTimer, 0);
}

#pragma stackfunction 1500
void wanson_engine_task(void *args)
{
    inference_state = STATE_EXPECTING_WAKEWORD;

    asr_ctx = asr_init(NULL, NULL);

    StreamBufferHandle_t input_queue = (StreamBufferHandle_t)args;
    TimerHandle_t inf_eng_tmr = xTimerCreate(
        "inf_eng_tmr",
        pdMS_TO_TICKS(appconfINFERENCE_RESET_DELAY_MS),
        pdFALSE,
        NULL,
        vInferenceTimerCallback);

    int32_t buf[appconfINFERENCE_SAMPLE_BLOCK_LENGTH] = {0};
    int16_t buf_short[WANSON_SAMPLES_PER_INFERENCE] = {0};

    asr_reset(asr_ctx);

    /* Alert other tile to start the audio pipeline */
    int dummy = 0;
    rtos_intertile_tx(intertile_ctx, appconfWANSON_READY_SYNC_PORT, &dummy, sizeof(dummy));

    asr_error_t asr_error;
    asr_result_t asr_result;
    asr_keyword_t asr_keyword;
    asr_command_t asr_command;

    size_t buf_short_index = 0;

#if appconfLOW_POWER_ENABLED
    lp_slave_user_not_active(lp_ctx, LP_SLAVE_LP_INT_TIMEOUT_HANDLER);
#endif
    while (1)
    {
        receive_audio_frames(input_queue, buf, buf_short, &buf_short_index);

        if (buf_short_index < WANSON_SAMPLES_PER_INFERENCE)
            continue;

        buf_short_index = 0; // reset the offset into the buffer of int16s.
                             // Note, we do not need to overlap the window of samples.
                             // This is handled in the ASR ports.
        
        asr_error = asr_process(asr_ctx, buf_short, WANSON_SAMPLES_PER_INFERENCE);
        if (asr_error != ASR_OK) continue; 

        asr_error = asr_get_result(asr_ctx, &asr_result);
        if (asr_error != ASR_OK) continue; 

        asr_keyword = asr_get_keyword(asr_ctx, asr_result.keyword_id);
        asr_command = asr_get_command(asr_ctx, asr_result.command_id);
        if (!IS_KEYWORD(asr_keyword) && !IS_COMMAND(asr_command)) continue; 

    #if appconfINFERENCE_RAW_OUTPUT
    #if appconfLOW_POWER_ENABLED
        hold_inf_state(inf_eng_tmr);
    #endif
        wanson_engine_process_asr_result(asr_keyword, asr_command);
    #else
        if (inference_state == STATE_EXPECTING_WAKEWORD && IS_KEYWORD(asr_keyword)) {
            led_indicate_listening();
            hold_inf_state(inf_eng_tmr);
            wanson_engine_process_asr_result(asr_keyword, asr_command);
            inference_state = STATE_EXPECTING_COMMAND;
        } else if (inference_state == STATE_EXPECTING_COMMAND && IS_COMMAND(asr_command)) {
            hold_inf_state(inf_eng_tmr);
            wanson_engine_process_asr_result(asr_keyword, asr_command);
            inference_state = STATE_PROCESSING_COMMAND;
        } else if (inference_state == STATE_EXPECTING_COMMAND && IS_KEYWORD(asr_keyword)) {
            hold_inf_state(inf_eng_tmr);
            wanson_engine_process_asr_result(asr_keyword, asr_command);
            // remain in STATE_EXPECTING_COMMAND state
        } else if (inference_state == STATE_PROCESSING_COMMAND && IS_KEYWORD(asr_keyword)) {
            hold_inf_state(inf_eng_tmr);
            wanson_engine_process_asr_result(asr_keyword, asr_command);
            inference_state = STATE_EXPECTING_COMMAND;
        } else if (inference_state == STATE_PROCESSING_COMMAND && IS_COMMAND(asr_command)) {
            hold_inf_state(inf_eng_tmr);
            wanson_engine_process_asr_result(asr_keyword, asr_command);
            // remain in STATE_PROCESSING_COMMAND state
        }
    #endif
    }
}

#endif /* ON_TILE(INFERENCE_TILE_NO) */
