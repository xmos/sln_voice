// Copyright (c) 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public License: Version 1

#ifndef LOW_POWER_AUDIO_BUFFER_H_
#define LOW_POWER_AUDIO_BUFFER_H_


/* System headers */
#include <stdint.h>
#include <stddef.h>
#include <platform.h>
#include <xs1.h>

/* App headers */
#include "app_conf.h"
#include "asr.h"

#define LOW_POWER_AUDIO_BUFFER_ENABLED ( \
    appconfAUDIO_PIPELINE_BUFFER_ENABLED && \
    ON_TILE(AUDIO_PIPELINE_TILE_NO) )

/**
 * Enqueue audio samples into a ring buffer. Oldest data will be overwritten.
 *
 * \param samples       The pointer to the samples to enqueue.
 * \param num_samples   The number of samples to enqueue.
 */
void low_power_audio_buffer_enqueue(asr_sample_t *samples, size_t num_samples);

/**
 * Dequeue audio frames out of a ring buffer. These frames are sent onward to
 * the inference engine.
 *
 * \param num_frames    The requested number of frames to dequeue, where
 *                      one frame is appconfAUDIO_PIPELINE_FRAME_ADVANCE
 *                      samples.
 * \return              The number of samples actually dequeued from the buffer.
 */
uint32_t low_power_audio_buffer_dequeue(uint32_t num_packets);

#endif // LOW_POWER_AUDIO_BUFFER_H_
