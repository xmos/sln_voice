// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

/* STD headers */
#include <string.h>
#include <stdint.h>
#include <xcore/hwtimer.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "stream_buffer.h"

/* Library headers */
#include "generic_pipeline.h"

/* App headers */
#include "app_conf.h"
#include "audio_pipeline.h"
#include "audio_pipeline_dsp.h"
#include "platform/driver_instances.h"
#include "my_src.h"

#if appconfAUDIO_PIPELINE_FRAME_ADVANCE != 240
#error This pipeline is only configured for 240 frame advance
#endif

typedef struct
{
    /* data */
    rtos_osal_queue_t input_queue;
    rtos_osal_queue_t output_queue;
}pipeline_ctx_t;

static agc_stage_ctx_t DWORD_ALIGNED agc_stage_state = {};

static void stage_agc(frame_data_t *frame_data)
{
#if appconfAUDIO_PIPELINE_SKIP_AGC
#else
    int32_t DWORD_ALIGNED agc_output[appconfAUDIO_PIPELINE_FRAME_ADVANCE];
    configASSERT(AGC_FRAME_ADVANCE == appconfAUDIO_PIPELINE_FRAME_ADVANCE);

    agc_stage_state.md.vnr_flag = frame_data->vnr_pred_flag;
    agc_stage_state.md.aec_ref_power = frame_data->max_ref_energy;
    agc_stage_state.md.aec_corr_factor = frame_data->aec_corr_factor;

    agc_process_frame(
            &agc_stage_state.state,
            agc_output,
            frame_data->samples[0],
            &agc_stage_state.md);
    memcpy(frame_data->samples, agc_output, appconfAUDIO_PIPELINE_FRAME_ADVANCE * sizeof(int32_t));
#endif
}

static void audio_pipeline_input_mic(int32_t **mic_ptr, size_t frame_count)
{
    static int flushed;
    while (!flushed) {
        size_t received;
        received = rtos_mic_array_rx(mic_array_ctx,
                                     mic_ptr,
                                     frame_count,
                                     0);
        if (received == 0) {
            rtos_mic_array_rx(mic_array_ctx,
                              mic_ptr,
                              frame_count,
                              portMAX_DELAY);
            flushed = 1;
        }
    }

    /*
     * NOTE: ALWAYS receive the next frame from the PDM mics,
     * even if USB is the current mic source. The controls the
     * timing since usb_audio_recv() does not block and will
     * receive all zeros if no frame is available yet.
     */
    rtos_mic_array_rx(mic_array_ctx,
                      mic_ptr,
                      frame_count,
                      portMAX_DELAY);
}

static void audio_pipeline_input_i2s(int32_t *i2s_rx_data, size_t frame_count)
{
    size_t rx_count =
    rtos_i2s_rx(i2s_ctx,
                (int32_t*) i2s_rx_data,
                frame_count,
                portMAX_DELAY);
    xassert(rx_count == frame_count);

}

static void audio_pipeline_input_i(void *args)
{
    printf("SAMPLING_RATE_MULTIPLIER = %d\n", SAMPLING_RATE_MULTIPLIER);
    printf("MIC_ARRAY_SAMPLING_FREQ = %d\n", MIC_ARRAY_SAMPLING_FREQ);
    rtos_osal_queue_t *pipeline_in_queue = (rtos_osal_queue_t*)args;
    static uint32_t prev_in;
    prev_in = get_reference_time();
    for(;;)
    {
        frame_data_t *frame_data;

        frame_data = pvPortMalloc(sizeof(frame_data_t));
        memset(frame_data, 0x00, sizeof(frame_data_t));

        audio_pipeline_input_mic((int32_t **)frame_data->mic_samples_passthrough, appconfAUDIO_PIPELINE_FRAME_ADVANCE);

        int32_t tmp[appconfAUDIO_PIPELINE_FRAME_ADVANCE * SAMPLING_RATE_MULTIPLIER][appconfAUDIO_PIPELINE_CHANNELS];
        audio_pipeline_input_i2s(&tmp[0][0], appconfAUDIO_PIPELINE_FRAME_ADVANCE * SAMPLING_RATE_MULTIPLIER); // Receive at I2S sampling rate

        // Downsample
        int32_t downsampler_output[appconfAUDIO_PIPELINE_FRAME_ADVANCE][appconfAUDIO_PIPELINE_CHANNELS];
        stage_downsampler(tmp, downsampler_output);

        // Copy to the output array
        int32_t *tmpptr = (int32_t *)&frame_data->aec_reference_audio_samples[0][0];
        for (int i=0; i<appconfAUDIO_PIPELINE_FRAME_ADVANCE; i++) {
            /* ref is first */
            *(tmpptr + i) = downsampler_output[i][0];
            *(tmpptr + i + appconfAUDIO_PIPELINE_FRAME_ADVANCE) = downsampler_output[i][1];
        }

        uint32_t current_in = get_reference_time();
        //printuintln(current_in - prev_in);
        prev_in = current_in;

        frame_data->vnr_pred_flag = 0;

        memcpy(frame_data->samples, frame_data->mic_samples_passthrough, sizeof(frame_data->samples));
        //memcpy(frame_data->samples, frame_data->aec_reference_audio_samples, sizeof(frame_data->samples)); // For reference passthrough
        //memset(frame_data->samples, 0, sizeof(frame_data->samples));

        (void) rtos_osal_queue_send(pipeline_in_queue, &frame_data, RTOS_OSAL_WAIT_FOREVER);
    }
}

static int audio_pipeline_output_i(void *args)
{
    rtos_osal_queue_t *queue = (rtos_osal_queue_t*)args;
    for(;;)
    {
        frame_data_t *frame_data;
        (void) rtos_osal_queue_receive(queue, &frame_data, RTOS_OSAL_WAIT_FOREVER);

        /* I2S expects sample channel format */
        int32_t tmp[appconfAUDIO_PIPELINE_FRAME_ADVANCE][appconfAUDIO_PIPELINE_CHANNELS];
        int32_t *tmpptr = &frame_data->samples[0][0];
        for (int j=0; j<appconfAUDIO_PIPELINE_FRAME_ADVANCE; j++) {
            /* ASR output is first */
            tmp[j][0] = *(tmpptr+j);
            tmp[j][1] = *(tmpptr+j+appconfAUDIO_PIPELINE_FRAME_ADVANCE);
        }
        int32_t output[appconfAUDIO_PIPELINE_FRAME_ADVANCE * SAMPLING_RATE_MULTIPLIER][appconfAUDIO_PIPELINE_CHANNELS];
        stage_upsampler(tmp, output);

        rtos_i2s_tx(i2s_ctx,
                (int32_t*) output,
                appconfAUDIO_PIPELINE_FRAME_ADVANCE * SAMPLING_RATE_MULTIPLIER,
                portMAX_DELAY);

        rtos_osal_free(frame_data);
    }
}

static void agc_task(void *args)
{
    pipeline_ctx_t *pipeline_ctx = args;

    for(;;)
    {
        // Get pipeline input
        frame_data_t *frame_data;
        (void) rtos_osal_queue_receive(&pipeline_ctx->input_queue, &frame_data, RTOS_OSAL_WAIT_FOREVER);

        // Process
        stage_agc(frame_data);

        // Send pipeline output
        (void) rtos_osal_queue_send(&pipeline_ctx->output_queue, &frame_data, RTOS_OSAL_WAIT_FOREVER);
    }
}

void pipeline_init()
{
#if ON_TILE(1)
    // Initialise one ASRC instance
    agc_init(&agc_stage_state.state, &AGC_PROFILE_FIXED_GAIN);
    agc_stage_state.state.config.gain = f32_to_float_s32(500);

    pipeline_ctx_t *pipeline_ctx = rtos_osal_malloc(sizeof(pipeline_ctx_t));
    (void) rtos_osal_queue_create(&pipeline_ctx->input_queue, NULL, 2, sizeof(void *));
    (void) rtos_osal_queue_create(&pipeline_ctx->output_queue, NULL, 2, sizeof(void *));

    // Create pipeline input task
    (void) rtos_osal_thread_create(
        (rtos_osal_thread_t *) NULL,
        (char *) "Pipeline_input",
        (rtos_osal_entry_function_t) audio_pipeline_input_i,
        (void *) (&pipeline_ctx->input_queue),
        (size_t) RTOS_THREAD_STACK_SIZE(audio_pipeline_input_i),
        (unsigned int) appconfAUDIO_PIPELINE_TASK_PRIORITY);

    // Create pipeline output task
    (void) rtos_osal_thread_create(
        (rtos_osal_thread_t *) NULL,
        (char *) "Pipeline_output",
        (rtos_osal_entry_function_t) audio_pipeline_output_i,
        (void *) (&pipeline_ctx->output_queue),
        (size_t) RTOS_THREAD_STACK_SIZE(audio_pipeline_output_i),
        (unsigned int) appconfAUDIO_PIPELINE_TASK_PRIORITY);

    // Create the AGC task
    (void) rtos_osal_thread_create(
        (rtos_osal_thread_t *) NULL,
        (char *) "AGC",
        (rtos_osal_entry_function_t) agc_task,
        (void *) pipeline_ctx,
        (size_t) RTOS_THREAD_STACK_SIZE(agc_task),
        (unsigned int) appconfAUDIO_PIPELINE_TASK_PRIORITY);
#endif
}
