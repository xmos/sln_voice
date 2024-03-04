// Copyright 2021-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <platform.h>
#include <xs1.h>
#include <xcore/hwtimer.h>

#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"

#include "app_conf.h"
#include "platform/driver_instances.h"
#include "ww_model_runner/ww_model_runner.h"

#define ASR_CHANNEL             (0)
#define COMMS_CHANNEL           (1)

#if appconfWW_ENABLED
extern configSTACK_DEPTH_TYPE model_runner_manager_stack_size;

static StreamBufferHandle_t audio_stream = NULL;
#include "print.h"
void ww_audio_send(rtos_intertile_t *intertile_ctx,
                    size_t frame_count,
                    int32_t (*processed_audio_frame)[2])
{

    configASSERT(frame_count == appconfAUDIO_PIPELINE_FRAME_ADVANCE);

    int16_t ww_samples[appconfAUDIO_PIPELINE_FRAME_ADVANCE];

    for (int i = 0; i < frame_count; i++) {
        ww_samples[i] = (int16_t)(processed_audio_frame[i][ASR_CHANNEL] >> 16);
    }
    printintln(processed_audio_frame[0][ASR_CHANNEL] );
    printintln(ww_samples[0]);
    if(audio_stream != NULL) {
        if (xStreamBufferSend(audio_stream, ww_samples, sizeof(ww_samples), 0) != sizeof(ww_samples)) {
            rtos_printf("lost output samples for ww\n");
        }
    }

    /*for (int i = 0; i < appconfINTENT_SAMPLE_BLOCK_LENGTH; i++) {
        buf_short[(*buf_short_index)++] = buf[i] >> 16;
    }*/
}

void ww_task_create(unsigned priority)
{

    audio_stream = xStreamBufferCreate(appconfINTENT_FRAME_BUFFER_MULT * appconfAUDIO_PIPELINE_FRAME_ADVANCE,
                                       appconfWW_FRAMES_PER_INFERENCE);

    xTaskCreate((TaskFunction_t)model_runner_manager,
                "model_manager",
                model_runner_manager_stack_size,
                audio_stream,
                uxTaskPriorityGet(NULL),
                NULL);
}

void intent_engine_ready_sync(void)
{
    //return;
    int sync = 3456;
#if ON_TILE(WW_TILE_NO)
    size_t len = rtos_intertile_rx_len(intertile_ctx, appconfINTENT_ENGINE_READY_SYNC_PORT, RTOS_OSAL_WAIT_FOREVER);

    xassert(len == sizeof(sync));
    rtos_intertile_rx_data(intertile_ctx, &sync, sizeof(sync));

#else

    rtos_intertile_tx(intertile_ctx, appconfINTENT_ENGINE_READY_SYNC_PORT, &sync, sizeof(sync));

#endif
}

#endif
