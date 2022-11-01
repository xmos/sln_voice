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
#include "rtos_swmem.h"
#include "ssd1306_rtos_support.h"
#include "wanson_inf_eng.h"
#include "wanson_api.h"
#include "xcore_device_memory.h"
#include "gpio_ctrl/leds.h"

#define WANSON_SAMPLES_PER_INFERENCE    (2 * appconfINFERENCE_SAMPLE_BLOCK_LENGTH) 

typedef enum inference_state {
    STATE_EXPECTING_WAKEWORD,
    STATE_EXPECTING_COMMAND,
    STATE_PROCESSING_COMMAND
} inference_state_t;

static inference_state_t inference_state;

void vDisplayClearCallback(TimerHandle_t pxTimer)
{
    if ((inference_state == STATE_EXPECTING_COMMAND) || (inference_state == STATE_PROCESSING_COMMAND)) {
        wanson_engine_proc_keyword_result(NULL, 50);    /* 50 is a special id that will play the no longer listening for command sound */
        led_indicate_waiting();
    }
    inference_state = STATE_EXPECTING_WAKEWORD;
}

#pragma stackfunction 1500
void wanson_engine_task(void *args)
{
    assert(WANSON_SAMPLES_PER_INFERENCE == 480); // Wanson ASR engine expects 480 samples per inference

    inference_state = STATE_EXPECTING_WAKEWORD;

#if ON_TILE(0)
    // NOTE: The Wanson model uses the .SwMem_data attribute but no SwMem event handling code is required.
    //       This may cause xflash to whine if the compiler optimizes out the __swmem_address symbol.
    //       To work around this, we simply need to init the swmem.
    rtos_swmem_init(0);
#endif

    size_t model_file_size;
    model_file_size = model_file_init();
    if (model_file_size == 0) {
        rtos_printf("ERROR: Failed to load model file\n");
        vTaskDelete(NULL);
    }

    rtos_printf("Wanson init\n");
    Wanson_ASR_Init();
    rtos_printf("Wanson init done\n");

    StreamBufferHandle_t input_queue = (StreamBufferHandle_t)args;
    TimerHandle_t display_clear_timer = xTimerCreate(
        "disp_clr",
        pdMS_TO_TICKS(appconfINFERENCE_RESET_DELAY_MS),
        pdFALSE,
        NULL,
        vDisplayClearCallback);

    int32_t buf[appconfINFERENCE_SAMPLE_BLOCK_LENGTH] = {0};
    int16_t buf_short[WANSON_SAMPLES_PER_INFERENCE] = {0};

    /* Perform any initialization here */
#if 1   // domain doesn't do anything right now, 0 is both wakeup and asr
    rtos_printf("Wanson reset for wakeup\n");
    int ret = Wanson_ASR_Reset(0);
#else
    rtos_printf("Wanson reset for asr\n");
    int ret = Wanson_ASR_Reset(1);
#endif
    rtos_printf("Wanson reset ret: %d\n", ret);

    /* Alert other tile to start the audio pipeline */
    int dummy = 0;
    rtos_intertile_tx(intertile_ctx, appconfWANSON_READY_SYNC_PORT, &dummy, sizeof(dummy));

    char *text_ptr = NULL;
    int id = 0;
    size_t buf_short_index = 0;

    while (1)
    {
        /* Receive audio frames */
        uint8_t *buf_ptr = (uint8_t*)buf;
        size_t buf_len = appconfINFERENCE_SAMPLE_BLOCK_LENGTH * sizeof(int32_t);

        do {
            size_t bytes_rxed = xStreamBufferReceive(input_queue,
                                                     buf_ptr,
                                                     buf_len,
                                                     portMAX_DELAY);
            buf_len -= bytes_rxed;
            buf_ptr += bytes_rxed;
        } while(buf_len > 0);

        for (int i=0; i<appconfINFERENCE_SAMPLE_BLOCK_LENGTH; i++) {
            buf_short[buf_short_index++] = buf[i] >> 16;
        }

        if (buf_short_index >= WANSON_SAMPLES_PER_INFERENCE)
        {
            /* Perform inference here */
            ret = Wanson_ASR_Recog(buf_short, WANSON_SAMPLES_PER_INFERENCE, (const char **)&text_ptr, &id);

            if (ret) {
    #if appconfINFERENCE_RAW_OUTPUT
                wanson_engine_proc_keyword_result((const char **)&text_ptr, id);
    #else
                if (inference_state == STATE_EXPECTING_WAKEWORD && IS_WAKEWORD(id)) {
                    led_indicate_listening();
                    xTimerReset(display_clear_timer, 0);
                    wanson_engine_proc_keyword_result((const char **)&text_ptr, id);
                    inference_state = STATE_EXPECTING_COMMAND;
                } else if (inference_state == STATE_EXPECTING_COMMAND && IS_COMMAND(id)) {
                    xTimerReset(display_clear_timer, 0);
                    wanson_engine_proc_keyword_result((const char **)&text_ptr, id);
                    inference_state = STATE_PROCESSING_COMMAND;
                } else if (inference_state == STATE_EXPECTING_COMMAND && IS_WAKEWORD(id)) {
                    xTimerReset(display_clear_timer, 0);
                    wanson_engine_proc_keyword_result((const char **)&text_ptr, id);
                    // remain in STATE_EXPECTING_COMMAND state
                } else if (inference_state == STATE_PROCESSING_COMMAND && IS_WAKEWORD(id)) {
                    xTimerReset(display_clear_timer, 0);
                    wanson_engine_proc_keyword_result((const char **)&text_ptr, id);
                    inference_state = STATE_EXPECTING_COMMAND;
                } else if (inference_state == STATE_PROCESSING_COMMAND && IS_COMMAND(id)) {
                    xTimerReset(display_clear_timer, 0);
                    wanson_engine_proc_keyword_result((const char **)&text_ptr, id);
                    // remain in STATE_PROCESSING_COMMAND state
                }
    #endif
            }
    
            buf_short_index = 0; // reset the offest into the buffer of int16s.  
                                 // Note, we do not need to overlap the window of samples.
                                 // This is handled in the Wanson ASR engine.
        }
    }
}
