// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef LOW_POWER_AUDIO_BUFFER_H_
#define LOW_POWER_AUDIO_BUFFER_H_


/* System headers */
#include <stdint.h>
#include <stddef.h>
#include <platform.h>
#include <xs1.h>

/* App headers */
#include "app_conf.h"

#define LOW_POWER_AUDIO_BUFFER_ENABLED ( \
    appconfLOW_POWER_ENABLED && \
    appconfAUDIO_PIPELINE_BUFFER_ENABLED && \
    ON_TILE(AUDIO_PIPELINE_TILE_NO) )

/**
 * Enqueue audio frames into a ring buffer. Oldest data will be overwritten.
 *
 * \param frames        The point to the frames to enqueue.
 * \param num_frames    The number of frames to enqueue.
 */
void low_power_audio_buffer_enqueue(int32_t *frames, size_t num_frames);

/**
 * Dequeue audio frames out of a ring buffer. These frames are sent onward to
 * the inference engine.
 *
 * \param num_frames    The requested number of frames to dequeue.
 * \return              The number of samples actually dequeued from the buffer.
 */
uint32_t low_power_audio_buffer_dequeue(uint32_t num_frames);

#endif // LOW_POWER_AUDIO_BUFFER_H_
