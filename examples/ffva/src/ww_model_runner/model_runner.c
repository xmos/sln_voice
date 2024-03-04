// Copyright 2021-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <platform.h>
#include <xs1.h>
#include <xcore/hwtimer.h>

#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"
#include "device_memory_impl.h"
#include "asr.h"

#include "app_conf.h"
#include "platform/driver_instances.h"
#include "ww_model_runner/ww_model_runner.h"

#if ASR_SENSORY
    #define IS_KEYWORD(id)    (id == 17)
    #define IS_COMMAND(id)    (id > 0 && id != 17)
#elif ASR_CYBERON
    #define IS_KEYWORD(id)    (id == 1)
    #define IS_COMMAND(id)    (id >= 2)
#else
#error "Model has to be either Sensory or Cyberon"
#endif

#define SAMPLES_PER_ASR                 (appconfINTENT_SAMPLE_BLOCK_LENGTH)

configSTACK_DEPTH_TYPE model_runner_manager_stack_size = 287;
// SEARCH model file is specified in the CMakeLists SENSORY_COMMAND_SEARCH_SOURCE_FILE variable
#ifdef COMMAND_SEARCH_SOURCE_FILE
extern const unsigned short gs_grammarLabel[];
void* grammar = (void*)gs_grammarLabel;
#else
void* grammar = NULL;
#endif
#include "print.h"
// Model file is in flash at the offset specified in the CMakeLists
// QSPI_FLASH_MODEL_START_ADDRESS variable.  The XS1_SWMEM_BASE value needs
// to be added so the address in in the SwMem range.
uint16_t *model = (uint16_t *) (XS1_SWMEM_BASE + QSPI_FLASH_MODEL_START_ADDRESS);

static asr_port_t asr_ctx;
static devmem_manager_t devmem_ctx;

#pragma stackfunction 1000
void model_runner_manager(void *args)
{
    StreamBufferHandle_t input_queue = (StreamBufferHandle_t)args;

    //int16_t buf[appconfWW_FRAMES_PER_INFERENCE];

    /* Perform any initialization here */
    //intent_state = STATE_EXPECTING_WAKEWORD;

    /*TimerHandle_t int_eng_tmr = xTimerCreate(
        "int_eng_tmr",
        pdMS_TO_TICKS(appconfINTENT_RESET_DELAY_MS),
        pdFALSE,
        NULL,
        vIntentTimerCallback);
    */
    /* Alert other tile to start the audio pipeline */
    intent_engine_ready_sync();
    devmem_init(&devmem_ctx);
    printf("Call asr_init(). model = 0x%x, grammar = 0x%x\n", (unsigned int) model, (unsigned int) grammar);
    asr_ctx = asr_init((int32_t *)model, (int32_t *)grammar, &devmem_ctx);

    int16_t buf[appconfINTENT_SAMPLE_BLOCK_LENGTH] = {0};
    //int16_t buf_short[SAMPLES_PER_ASR] = {0};

    asr_reset(asr_ctx);

    //size_t buf_short_index = 0;
    asr_error_t asr_error;
    asr_result_t asr_result;
    int word_id;

    while (1)
    {
        /* Receive audio frames */
        uint8_t *buf_ptr = (uint8_t*)buf;
        size_t buf_len = appconfWW_FRAMES_PER_INFERENCE * sizeof(int16_t);
        do {
            size_t bytes_rxed = xStreamBufferReceive(input_queue,
                                                     buf_ptr,
                                                     buf_len,
                                                     portMAX_DELAY);
            buf_len -= bytes_rxed;
            buf_ptr += bytes_rxed;
        } while(buf_len > 0);

        //for (int i = 0; i < appconfINTENT_SAMPLE_BLOCK_LENGTH; i++) {
        //    buf_short[buf_short_index++] = buf[i] >> 16;
        //}
        //if (buf_short_index < SAMPLES_PER_ASR)
        //    continue;

        //buf_short_index = 0; // reset the offset into the buffer of int16s.
                             // Note, we do not need to overlap the window of samples.
                             // This is handled in the ASR ports.

        // this application does not support barge-in
        //   so, we need to check if an audio response is playing and skip to the next
        //   audio frame because the playback may trigger the ASR.
        //if (intent_handler_response_playing()) continue;
        asr_error = asr_process(asr_ctx, buf, SAMPLES_PER_ASR);

        if (asr_error == ASR_EVALUATION_EXPIRED) {
            //led_indicate_end_of_eval();
            continue;
        }
        if (asr_error != ASR_OK) continue;

        asr_error = asr_get_result(asr_ctx, &asr_result);
        if (asr_error != ASR_OK) continue;

        word_id = asr_result.id;

        if (!IS_KEYWORD(word_id) && !IS_COMMAND(word_id)) continue;

/*
    #if appconfINTENT_RAW_OUTPUT
        intent_engine_process_asr_result(word_id);
    #else
        if (intent_state == STATE_EXPECTING_WAKEWORD && IS_KEYWORD(word_id)) {
            led_indicate_listening();
            xTimerStart(int_eng_tmr, 0);
            intent_engine_process_asr_result(word_id);
            intent_state = STATE_EXPECTING_COMMAND;
        } else if (intent_state == STATE_EXPECTING_COMMAND && IS_COMMAND(word_id)) {
            xTimerReset(int_eng_tmr, 0);
            intent_engine_process_asr_result(word_id);
            intent_state = STATE_PROCESSING_COMMAND;
        } else if (intent_state == STATE_EXPECTING_COMMAND && IS_KEYWORD(word_id)) {
            xTimerReset(int_eng_tmr, 0);
            intent_engine_process_asr_result(word_id);
            // remain in STATE_EXPECTING_COMMAND state
        } else if (intent_state == STATE_PROCESSING_COMMAND && IS_KEYWORD(word_id)) {
            xTimerReset(int_eng_tmr, 0);
            intent_engine_process_asr_result(word_id);
            intent_state = STATE_EXPECTING_COMMAND;
        } else if (intent_state == STATE_PROCESSING_COMMAND && IS_COMMAND(word_id)) {
            xTimerReset(int_eng_tmr, 0);
            intent_engine_process_asr_result(word_id);
            // remain in STATE_PROCESSING_COMMAND state
        }
    #endif
    }
*/
        /* Perform inference here */
        // rtos_printf("inference\n");
    }
}
