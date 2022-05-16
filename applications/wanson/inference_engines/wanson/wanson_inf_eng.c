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

/* in it's own thread for debug purposes right now... */
static void wanson_eng_thread(void *arg)
{
    (void) arg;

    rtos_printf("wanson init uses stack: %d\n", RTOS_THREAD_STACK_SIZE(wanson_eng_thread));
    Wanson_ASR_Init();

    while(1) {
        rtos_printf("****wanson init done\n");
    }
}

#pragma stackfunction 2000
void wanson_engine_task(void *args)
{
    xTaskCreate((TaskFunction_t)wanson_eng_thread,
            "wanson_eng_thread",
            RTOS_THREAD_STACK_SIZE(wanson_eng_thread),
            NULL,
            uxTaskPriorityGet(NULL)+1,
            NULL);
    while(1) {;}

    StreamBufferHandle_t input_queue = (StreamBufferHandle_t)args;

    int32_t buf[appconfINFERENCE_FRAMES_PER_INFERENCE] = {0};
    int16_t buf_short[2 * appconfINFERENCE_FRAMES_PER_INFERENCE] = {0};

    /* Perform any initialization here */
    // rtos_printf("create wanson task\n");
    // wanson_task_create(uxTaskPriorityGet(NULL), NULL);   // in the API but doesnt exist
    vTaskDelay(pdMS_TO_TICKS(5000));    // dummy time so the other thread calling wanson's init has time to finish reading in from swmem

#if 1
    rtos_printf("wanson reset for wakeup\n");
    int ret = Wanson_ASR_Reset(0);
#else
    rtos_printf("wanson reset for asr\n");
    int ret = Wanson_ASR_Reset(1);
#endif
    rtos_printf("wanson reset ret: %d\n", ret);

    char **text_ptr = NULL;
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
        ret = Wanson_ASR_Recog(buf_short, appconfINFERENCE_FRAMES_PER_INFERENCE, text_ptr, &id);
        if (ret) {
            rtos_printf("inference got ret %d and %s\n", ret, text_ptr); // wanson doesnt seem to return a valid string ptr
        }

        /* Push back history */
        for (int i=0; i<appconfINFERENCE_FRAMES_PER_INFERENCE; i++) {
            buf_short[i] = buf_short[i + appconfINFERENCE_FRAMES_PER_INFERENCE];
        }
    }
}
