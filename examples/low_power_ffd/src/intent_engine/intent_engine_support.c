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

/* App headers */
#include "app_conf.h"
#include "platform/driver_instances.h"
#include "intent_engine/intent_engine.h"

#if ON_TILE(ASR_TILE_NO)

static StreamBufferHandle_t samples_to_engine_stream_buf = 0;

#endif /* ON_TILE(ASR_TILE_NO) */

#if ON_TILE(AUDIO_PIPELINE_TILE_NO)

void intent_engine_samples_send_remote(
        rtos_intertile_t *intertile,
        size_t frame_count,
        asr_sample_t *processed_audio_frame)
{
    configASSERT(frame_count == appconfAUDIO_PIPELINE_FRAME_ADVANCE);

    rtos_intertile_tx(intertile,
                      appconfINTENT_MODEL_RUNNER_SAMPLES_PORT,
                      processed_audio_frame,
                      sizeof(asr_sample_t) * frame_count);
}

#else /* ON_TILE(AUDIO_PIPELINE_TILE_NO) */

static void intent_engine_intertile_samples_in_task(void *arg)
{
    (void) arg;

    for (;;) {
        asr_sample_t samples[appconfAUDIO_PIPELINE_FRAME_ADVANCE];
        size_t bytes_received;

        bytes_received = rtos_intertile_rx_len(
                intertile_ap_ctx,
                appconfINTENT_MODEL_RUNNER_SAMPLES_PORT,
                portMAX_DELAY);

        xassert(bytes_received == sizeof(samples));

        rtos_intertile_rx_data(
                intertile_ap_ctx,
                samples,
                bytes_received);

        if (xStreamBufferSend(samples_to_engine_stream_buf, samples, sizeof(samples), 0) != sizeof(samples)) {
            rtos_printf("lost output samples for intent\n");
        }
    }
}

void intent_engine_stream_buf_reset(void)
{
    if (samples_to_engine_stream_buf)
        while (xStreamBufferReset(samples_to_engine_stream_buf) == pdFAIL)
            vTaskDelay(pdMS_TO_TICKS(1));
}

void intent_engine_intertile_task_create(uint32_t priority)
{
    samples_to_engine_stream_buf = xStreamBufferCreate(
                                           appconfINTENT_FRAME_BUFFER_MULT * appconfAUDIO_PIPELINE_FRAME_ADVANCE,
                                           appconfINTENT_SAMPLE_BLOCK_LENGTH);

    xTaskCreate((TaskFunction_t)intent_engine_intertile_samples_in_task,
                "int_intertile_rx",
                RTOS_THREAD_STACK_SIZE(intent_engine_intertile_samples_in_task),
                NULL,
                priority-1,
                NULL);
    xTaskCreate((TaskFunction_t)intent_engine_task,
                "intent_eng",
                RTOS_THREAD_STACK_SIZE(intent_engine_task),
                samples_to_engine_stream_buf,
                uxTaskPriorityGet(NULL),
                NULL);
}

#endif /* ON_TILE(AUDIO_PIPELINE_TILE_NO) */
