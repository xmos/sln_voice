// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#pragma once

#include <stdint.h>
//#include "xccompat.h"
//#include "app_config.h"
#include "rtos_mic_array.h"
//#include "util/audio_buffer.h"


C_API_START

//void board_dac3101_init();

//void app_i2s_task( audio_ring_buffer_t* app_context );

/**
 * This thread consumes one sample at a time from the mic array via
 * `c_sample_in` and outputs blocks of 32 PDM samples (as a `uint32_t`) via
 * `c_sample_out`.
 * 
 * The thread upsamples by a factor of 192, applies a third order delta-sigma
 * modulator to produce a bipolar signal and condenses that signal such that
 * each group of 32 bipolar samples becomes a single `uint32_t`.
 * 
 * Less significant bits are earlier in time (as expected by port hardware), 
 * a bit value of 1 corresponds to a -1 sample and a bit value of 0 corresponds
 * to a +1 sample.
 * 
 * Each sample sent to this task through `c_sample_in` yields `6` output words
 * through `c_sample_out`.
 * 
 * Note that this thread sends each PDM block as soon as it is available, rather
 * than waiting for all 6 blocks to be computed, so a subsequent task receiving
 * PDM blocks through `c_sample_out` should (usually) process each received word
 * as it is received, rather than waiting to receive 6 before processing.
 */
C_API
void pcm2pdm(int32_t sample[]);

C_API
//void pcm_to_pdm_full(
//    chanend_t c_sample_in,
//    streaming_channel_t c_sample_out);

void pcm_to_pdm_full(
    chanend_t c_sample_in,
    chanend_t c_sample_out);
/**
 * Upsample a PCM32 stream by a factor of 192.
 */
C_API
void upsample_x192_thread(
    chanend_t c_sample_in,
    streaming_channel_t c_sample_out);

/**
 * Appy a third order delta-sigma modulator to a stream of samples and output
 * blocks of PDM data. Every 32 inputs should result in one block of PDM data
 * output through `c_sample_out`. Output format is same as `pcm_to_pdm_full()`
 */
C_API
//void pcm_to_pdm(streaming_channel_t c_sample_in,
//                streaming_channel_t c_sample_out);
void pcm_to_pdm(chanend_t c_sample_in,
                chanend_t c_sample_out);
/**
 * Use mic array decimator to downsample a stream of PDM data by a factor of 
 * 192 to produce a PCM32 stream.
 * 
 * Blocks of 32 PDM samples are fed in as uint32_t, same as the
 * `pcm_to_pdm_full()` output.
 */
C_API
//void pdm_to_pcm(streaming_channel_t c_sample_in,
//                streaming_channel_t c_sample_out);

void pdm_to_pcm(chanend_t c_sample_in,
                chanend_t c_sample_out);

C_API_END
