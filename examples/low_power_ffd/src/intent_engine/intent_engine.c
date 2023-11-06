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

/* App headers */
#include "app_conf.h"
#include "asr.h"
#include "device_memory_impl.h"
#include "gpio_ctrl/leds.h"
#include "intent_engine/intent_engine.h"
#include "platform/driver_instances.h"
#include "power/power_state.h"
#include "power/power_control.h"

#if ON_TILE(ASR_TILE_NO)

// This define is referenced by the model source/header files.
#ifndef ALIGNED
#define ALIGNED(x) __attribute__ ((aligned((x))))
#endif

#define SEARCH_VAR gs_command_grammarLabel

#ifdef COMMAND_SEARCH_HEADER_FILE
#include COMMAND_SEARCH_HEADER_FILE
#endif

#ifdef COMMAND_SEARCH_SOURCE_FILE
#include COMMAND_SEARCH_SOURCE_FILE
#else
extern const unsigned short SEARCH_VAR[];
#endif

#define IS_COMMAND(id)      ((id) > 0)
#define SAMPLES_PER_ASR     (appconfINTENT_SAMPLE_BLOCK_LENGTH)

typedef enum intent_power_state {
    STATE_REQUESTING_LOW_POWER,
    STATE_ENTERING_LOW_POWER,
    STATE_ENTERED_LOW_POWER,
    STATE_EXITING_LOW_POWER,
    STATE_EXITED_LOW_POWER
} intent_power_state_t;

enum timeout_event {
    TIMEOUT_EVENT_NONE = 0,
    TIMEOUT_EVENT_INTENT = 1
};

// Sensory NET model file is in flash at the offset specified in the CMakeLists
// QSPI_FLASH_MODEL_START_ADDRESS variable.  The XS1_SWMEM_BASE value needs
// to be added so the address in in the SwMem range.
uint16_t *dnn_netLabel = (uint16_t *) (XS1_SWMEM_BASE + QSPI_FLASH_MODEL_START_ADDRESS);

static asr_port_t asr_ctx;
static devmem_manager_t devmem_ctx;

static intent_power_state_t intent_power_state;
static uint8_t requested_full_power;

static uint32_t timeout_event = TIMEOUT_EVENT_NONE;
static uint32_t asr_halted = 0;

static void vIntentTimerCallback(TimerHandle_t pxTimer);
static void receive_audio_frames(StreamBufferHandle_t input_queue, asr_sample_t *buf);
static void timeout_event_handler(TimerHandle_t pxTimer);
static void hold_intent_state(TimerHandle_t pxTimer);
static void hold_full_power(TimerHandle_t pxTimer);
static uint8_t low_power_handler(TimerHandle_t pxTimer, asr_sample_t *buf);
static void wait_for_keyword_queue_completion(void);

static void vIntentTimerCallback(TimerHandle_t pxTimer)
{
    timeout_event |= TIMEOUT_EVENT_INTENT;
}

static void receive_audio_frames(StreamBufferHandle_t input_queue, asr_sample_t *buf)
{
    uint8_t *buf_ptr = (uint8_t*)buf;
    size_t buf_len = SAMPLES_PER_ASR * sizeof(asr_sample_t);

    do {
        size_t bytes_rxed = xStreamBufferReceive(input_queue,
                                                 buf_ptr,
                                                 buf_len,
                                                 portMAX_DELAY);
        buf_len -= bytes_rxed;
        buf_ptr += bytes_rxed;
    } while (buf_len > 0);
}

static void timeout_event_handler(TimerHandle_t pxTimer)
{
    if (timeout_event & TIMEOUT_EVENT_INTENT) {
        timeout_event &= ~TIMEOUT_EVENT_INTENT;
        if (intent_engine_low_power_ready()) {
            intent_power_state = STATE_REQUESTING_LOW_POWER;
            power_control_req_low_power();
        } else {
            hold_full_power(pxTimer);
        }
    }
}

static void hold_intent_state(TimerHandle_t pxTimer)
{
    xTimerStop(pxTimer, 0);
    xTimerChangePeriod(pxTimer, pdMS_TO_TICKS(appconfINTENT_RESET_DELAY_MS), 0);
    timeout_event = TIMEOUT_EVENT_NONE;
    xTimerReset(pxTimer, 0);
}

static void wait_for_keyword_queue_completion(void)
{
    const TickType_t poll_interval = pdMS_TO_TICKS(100);

    while (!intent_engine_low_power_ready()) {
        vTaskDelay(poll_interval);
    }
}

static void hold_full_power(TimerHandle_t pxTimer)
{
    xTimerStop(pxTimer, 0);
    xTimerChangePeriod(pxTimer, pdMS_TO_TICKS(appconfLOW_POWER_INHIBIT_MS), 0);
    timeout_event = TIMEOUT_EVENT_NONE;
    xTimerReset(pxTimer, 0);
}

static uint8_t low_power_handler(TimerHandle_t pxTimer, asr_sample_t *buf)
{
    uint8_t low_power = 0;

    switch (intent_power_state) {
    case STATE_REQUESTING_LOW_POWER:
        low_power = 1;
        // Wait here until other tile accepts/rejects the request.
        if (requested_full_power) {
            requested_full_power = 0;
            // Aborting low power transition.
            intent_power_state = STATE_EXITING_LOW_POWER;
        }
        break;
    case STATE_ENTERING_LOW_POWER:
        /* Prior to entering this state, the other tile is to cease pushing
         * samples to the stream buffer. */
        memset(buf, 0, SAMPLES_PER_ASR);
        intent_engine_stream_buf_reset();
        wait_for_keyword_queue_completion();
        intent_power_state = STATE_ENTERED_LOW_POWER;
        break;
    case STATE_ENTERED_LOW_POWER:
        low_power = 1;
        if (requested_full_power) {
            requested_full_power = 0;
            /* Reset ASR here instead of STATE_EXITING_LOW_POWER to avoid
             * a reset when STATE_REQUESTING_LOW_POWER is NAK'd, suggesting
             * that a wake word may have been spoken. */
            asr_reset(asr_ctx);
            intent_engine_keyword_queue_reset();
            intent_power_state = STATE_EXITING_LOW_POWER;
        }
        break;
    case STATE_EXITING_LOW_POWER:
        hold_intent_state(pxTimer);
        led_indicate_idle();
        intent_power_state = STATE_EXITED_LOW_POWER;
        break;
    case STATE_EXITED_LOW_POWER:
    default:
        break;
    }

    return low_power;
}

void intent_engine_halt(void)
{
    requested_full_power = 1;
    asr_halted = 1;
}

void intent_engine_full_power_request(void)
{
    requested_full_power = 1;
}

void intent_engine_low_power_accept(void)
{
    // The request has been accepted proceed with finalizing low power transition.
    intent_power_state = STATE_ENTERING_LOW_POWER;
}

#pragma stackfunction 1000
void intent_engine_task(void *args)
{
    StreamBufferHandle_t input_queue = (StreamBufferHandle_t)args;
    asr_sample_t buf[SAMPLES_PER_ASR] = {0};
    asr_error_t asr_error = ASR_OK;
    asr_result_t asr_result;

    TimerHandle_t int_eng_tmr = xTimerCreate(
        "int_eng_tmr",
        pdMS_TO_TICKS(appconfINTENT_RESET_DELAY_MS),
        pdFALSE,
        NULL,
        vIntentTimerCallback);

    devmem_init(&devmem_ctx);
    asr_ctx = asr_init((int32_t *)dnn_netLabel, (int32_t *)SEARCH_VAR, &devmem_ctx);

    /* Immediately signal intent timeout, to start a request to enter low power.
     * This is to help prevent commands from being detected at startup
     * (without a wake-word event). */
    timeout_event |= TIMEOUT_EVENT_INTENT;
    requested_full_power = 0;

    /* Reset the ASR and set LED indication, in case the other tile NAKs the
     * low power request at startup. */
    asr_reset(asr_ctx);
    led_indicate_idle();

    /* Alert other tile to start the audio pipeline */
    intent_engine_ready_sync();
    int run_asr = 1;

    while (1)
    {
        timeout_event_handler(int_eng_tmr);

        if (low_power_handler(int_eng_tmr, buf)) {
            // Low power, processing stopped.
            continue;
        }

        receive_audio_frames(input_queue, buf);

        if (run_asr == 0)
            continue;

        asr_error = asr_process(asr_ctx, buf, SAMPLES_PER_ASR);

        if (asr_error == ASR_OK) {
            asr_error = asr_get_result(asr_ctx, &asr_result);
        }

        if (asr_error == ASR_EVALUATION_EXPIRED || asr_halted) {
            xTimerStop(int_eng_tmr, 0);
            timeout_event = TIMEOUT_EVENT_NONE;
            asr_halted = 1;
            run_asr = 0;
            led_indicate_end_of_eval();
            debug_printf("ASR evaluation ended. Restart device to restore operation.\n");
        } else if (asr_error != ASR_OK) {
            debug_printf("ASR error on tile %d: %d\n", THIS_XCORE_TILE, asr_error);
        } else if (IS_COMMAND(asr_result.id)) {
            hold_intent_state(int_eng_tmr);
            intent_engine_process_asr_result(asr_result.id);
        }
    }
}

#endif /* ON_TILE(ASR_TILE_NO) */

void intent_engine_ready_sync(void)
{
    int sync = 0;
#if ON_TILE(AUDIO_PIPELINE_TILE_NO)
    size_t len = rtos_intertile_rx_len(intertile_ctx, appconfINTENT_ENGINE_READY_SYNC_PORT, RTOS_OSAL_WAIT_FOREVER);
    xassert(len == sizeof(sync));
    rtos_intertile_rx_data(intertile_ctx, &sync, sizeof(sync));
#else
    rtos_intertile_tx(intertile_ctx, appconfINTENT_ENGINE_READY_SYNC_PORT, &sync, sizeof(sync));
#endif
}

