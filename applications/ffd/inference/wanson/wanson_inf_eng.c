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
#include "wanson_api.h"

#include "ssd1306_rtos_support.h"

#pragma stackfunction 1200
void wanson_engine_task(void *args)
{
    rtos_printf("Wanson init\n");
    Wanson_ASR_Init();
    rtos_printf("Wanson init done\n");

    StreamBufferHandle_t input_queue = (StreamBufferHandle_t)args;

    int32_t buf[appconfINFERENCE_FRAMES_PER_INFERENCE] = {0};
    int16_t buf_short[2 * appconfINFERENCE_FRAMES_PER_INFERENCE] = {0};

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
    while (1)
    {
        /* Receive audio frames */
        uint8_t *buf_ptr = (uint8_t*)buf;
        size_t buf_len = appconfINFERENCE_FRAMES_PER_INFERENCE * sizeof(int32_t);
        do {
            size_t bytes_rxed = xStreamBufferReceive(input_queue,
                                                     buf_ptr,
                                                     buf_len,
                                                     portMAX_DELAY);
            buf_len -= bytes_rxed;
            buf_ptr += bytes_rxed;
        } while(buf_len > 0);

        /* Set second half of frame, as first contains last sample, also downshift for model format */
        for (int i=0; i<appconfINFERENCE_FRAMES_PER_INFERENCE; i++) {
            buf_short[i + appconfINFERENCE_FRAMES_PER_INFERENCE] = buf[i] >> 16;
        }

        /* Perform inference here */
        ret = Wanson_ASR_Recog(buf_short, appconfINFERENCE_FRAMES_PER_INFERENCE, (const char **)&text_ptr, &id);

        if (ret) {
            rtos_printf("inference got ret %d: %s %d\n", ret, text_ptr, id);
            // some temporary fixes to the strings returned
            switch (id) {
                case 200:
                    // fix capital "On"
                    ssd1306_display_ascii_to_bitmap("Switch on the TV\0");
                    break;
                case 420:
                    // fix lower case "speed"
                    // fix word wrapping
                    ssd1306_display_ascii_to_bitmap("Speed up the   fan\0");
                    break;
                case 430:
                    // fix lower case "slow"
                    ssd1306_display_ascii_to_bitmap("Slow down the fan\0");
                    break;
                case 440:
                    // fix lower case "set"
                    // fix word wrapping
                    ssd1306_display_ascii_to_bitmap("Set higher    temperature\0");
                    break; 
                case 450:
                    // fix lower case "set"
                    // fix word wrapping
                    ssd1306_display_ascii_to_bitmap("Set lower     temperature\0");
                    break;
                default:
                    ssd1306_display_ascii_to_bitmap(text_ptr);
            }
        }

        /* Push back history */
        for (int i=0; i<appconfINFERENCE_FRAMES_PER_INFERENCE; i++) {
            buf_short[i] = buf_short[i + appconfINFERENCE_FRAMES_PER_INFERENCE];
        }
    }
}
