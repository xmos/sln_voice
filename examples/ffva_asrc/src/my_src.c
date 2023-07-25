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
#include <src.h>

void stage_upsampler(
    int32_t (*frame_data)[appconfAUDIO_PIPELINE_CHANNELS],
    int32_t (*output)[appconfAUDIO_PIPELINE_CHANNELS]
    )
{
    if (SAMPLING_RATE_MULTIPLIER == 3) {
        static int32_t __attribute__((aligned (8))) src_data[appconfAUDIO_PIPELINE_CHANNELS][SRC_FF3V_FIR_TAPS_PER_PHASE];

        //uint32_t start = get_reference_time();
        for (int i = 0; i < appconfAUDIO_PIPELINE_FRAME_ADVANCE ; i++) {
            for (int j = 0; j < appconfAUDIO_PIPELINE_CHANNELS; j++) {
                output[SAMPLING_RATE_MULTIPLIER*i + 0][j] = src_us3_voice_input_sample(src_data[j], src_ff3v_fir_coefs[2], (int32_t)frame_data[i][j]);
                output[SAMPLING_RATE_MULTIPLIER*i + 1][j] = src_us3_voice_get_next_sample(src_data[j], src_ff3v_fir_coefs[1]);
                output[SAMPLING_RATE_MULTIPLIER*i + 2][j] = src_us3_voice_get_next_sample(src_data[j], src_ff3v_fir_coefs[0]);
            }
        }
        //uint32_t end = get_reference_time();
        //printuintln(end-start);
    }
    else if(SAMPLING_RATE_MULTIPLIER == 1)
    {
        // Copy input to output
        for (int i = 0; i < appconfAUDIO_PIPELINE_FRAME_ADVANCE ; i++) {
            for (int j = 0; j < appconfAUDIO_PIPELINE_CHANNELS; j++) {
                output[i][j] = frame_data[i][j];
            }
        }
    }
    else
    {
        printf("Unsupported SAMPLING_RATE_MULTIPLIER %d\n", SAMPLING_RATE_MULTIPLIER);
        xassert(0);
    }
}

void stage_downsampler(
    int32_t (*input)[appconfAUDIO_PIPELINE_CHANNELS],
    int32_t (*output)[appconfAUDIO_PIPELINE_CHANNELS]
    )
{
    if (SAMPLING_RATE_MULTIPLIER == 3) {
        static int32_t __attribute__((aligned (8))) src_data[appconfAUDIO_PIPELINE_CHANNELS][SRC_FF3V_FIR_NUM_PHASES][SRC_FF3V_FIR_TAPS_PER_PHASE];
        for (int i = 0; i < appconfAUDIO_PIPELINE_FRAME_ADVANCE / SAMPLING_RATE_MULTIPLIER; i++) {
            for (int j = 0; j < appconfAUDIO_PIPELINE_CHANNELS; j++) {
                int64_t sum = 0;
                sum = src_ds3_voice_add_sample(sum, src_data[j][0], src_ff3v_fir_coefs[0], input[SAMPLING_RATE_MULTIPLIER*i + 0][j]);
                sum = src_ds3_voice_add_sample(sum, src_data[j][1], src_ff3v_fir_coefs[1], input[SAMPLING_RATE_MULTIPLIER*i + 1][j]);
                output[i][j] = src_ds3_voice_add_final_sample(sum, src_data[j][2], src_ff3v_fir_coefs[2], input[SAMPLING_RATE_MULTIPLIER*i + 2][j]);
            }
        }
    }
    else if(SAMPLING_RATE_MULTIPLIER == 1)
    {
        for (int i = 0; i < appconfAUDIO_PIPELINE_FRAME_ADVANCE / SAMPLING_RATE_MULTIPLIER; i++) {
            for (int j = 0; j < appconfAUDIO_PIPELINE_CHANNELS; j++) {
                output[i][j] = input[i][j];
            }
        }
    }
    else
    {
        printf("Unsupported SAMPLING_RATE_MULTIPLIER %d\n", SAMPLING_RATE_MULTIPLIER);
        xassert(0);
    }
}
