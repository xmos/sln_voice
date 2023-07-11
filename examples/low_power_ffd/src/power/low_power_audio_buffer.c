// Copyright (c) 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public License: Version 1

/* System headers */
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <platform.h>
#include <xs1.h>

/* App headers */
#include "app_conf.h"
#include "low_power_audio_buffer.h"
#include "asr.h"
#include "intent_engine.h"

#if LOW_POWER_AUDIO_BUFFER_ENABLED

typedef struct ring_buffer
{
    asr_sample_t * const buf;   // The head of the buffer where data is stored.
    const uint32_t size;        // Number of bytes in buffer.
    char *set_ptr;              // The pointer where to set data.
    char *get_ptr;              // The pointer where to get data.
    uint32_t count;             // The number of valid asr_sample_t entries in the buffer.
    uint8_t full;               // Indicates the buffer is full.
    uint8_t empty;              // Indicates the buffer is empty.
} ring_buffer_t;

asr_sample_t sample_buf[appconfAUDIO_PIPELINE_BUFFER_NUM_FRAMES * appconfAUDIO_PIPELINE_FRAME_ADVANCE] = {0};

/* Ring buffer to hold onto the latest audio samples while in low power mode.
 * This serves to capture the onset of speech that meet or exceed the trigger
 * threshold to exit low power and to provide a more complete ASR payload to
 * the inference engine. */
ring_buffer_t ring_buf = {
    sample_buf,
    sizeof(sample_buf),
    (char *)sample_buf,
    (char *)sample_buf,
    0,
    0,
    1
};

void low_power_audio_buffer_enqueue(asr_sample_t *samples, size_t num_samples)
{
    const uint32_t tail_addr = ((uint32_t)ring_buf.buf + ring_buf.size);

    assert(num_samples <= appconfAUDIO_PIPELINE_BUFFER_NUM_FRAMES * appconfAUDIO_PIPELINE_FRAME_ADVANCE);

    size_t total_bytes = num_samples * sizeof(asr_sample_t);
    size_t tail_bytes = ring_buf.size - ((uint32_t)ring_buf.set_ptr - (uint32_t)ring_buf.buf);

    if (tail_bytes > total_bytes)
        tail_bytes = total_bytes;

    uint32_t head_bytes = total_bytes - tail_bytes;

    memcpy(ring_buf.set_ptr, (char *)samples, tail_bytes);
    memcpy(ring_buf.buf, (char *)samples + tail_bytes, head_bytes);

    if (head_bytes)
        ring_buf.set_ptr = (char *)ring_buf.buf + head_bytes;
    else
        ring_buf.set_ptr += tail_bytes;

    if ((uint32_t)ring_buf.set_ptr >= tail_addr)
        ring_buf.set_ptr = (char *)ring_buf.buf;

    ring_buf.full = ((ring_buf.count + num_samples) >= (ring_buf.size / sizeof(asr_sample_t)));

    if (ring_buf.full) {
        ring_buf.get_ptr = ring_buf.set_ptr;
        ring_buf.count = (ring_buf.size / sizeof(asr_sample_t));
    } else {
        ring_buf.count += num_samples;
    }

    ring_buf.empty = (ring_buf.count == 0);
}

uint32_t low_power_audio_buffer_dequeue(uint32_t num_frames)
{
    uint32_t ret = 0;
    const uint32_t tail_addr = ((uint32_t)ring_buf.buf + ring_buf.size);

    if ((ring_buf.count < appconfAUDIO_PIPELINE_FRAME_ADVANCE) || (num_frames == 0)) {
        // No data to dequeue.
        return ret;
    }

    ring_buf.full = 0;

    uint32_t bufferred_audio_bytes = ring_buf.count * sizeof(asr_sample_t);
    int32_t tail_bytes = ring_buf.size - ((uint32_t)ring_buf.get_ptr - (uint32_t)ring_buf.buf);

    int32_t samples_to_dequeue =
        ((num_frames * appconfAUDIO_PIPELINE_FRAME_ADVANCE) > ring_buf.count) ?
        ring_buf.count :
        (num_frames * appconfAUDIO_PIPELINE_FRAME_ADVANCE);

    ring_buf.empty = ((ring_buf.count - samples_to_dequeue) == 0);
    ret = (uint32_t)samples_to_dequeue;

    if ((uint32_t)tail_bytes > bufferred_audio_bytes)
        tail_bytes = bufferred_audio_bytes;

    int32_t head_bytes = bufferred_audio_bytes - tail_bytes;

    while (tail_bytes > 0) {
        if (samples_to_dequeue <= 0)
            break;

        intent_engine_sample_push((asr_sample_t *)ring_buf.get_ptr, appconfAUDIO_PIPELINE_FRAME_ADVANCE);
        ring_buf.get_ptr += (appconfAUDIO_PIPELINE_FRAME_ADVANCE * sizeof(asr_sample_t));
        tail_bytes -= (appconfAUDIO_PIPELINE_FRAME_ADVANCE * sizeof(asr_sample_t));
        samples_to_dequeue -= appconfAUDIO_PIPELINE_FRAME_ADVANCE;
        ring_buf.count -= appconfAUDIO_PIPELINE_FRAME_ADVANCE;
    }

    if ((uint32_t)ring_buf.get_ptr >= tail_addr)
        ring_buf.get_ptr = (char *)ring_buf.buf;

    if (head_bytes == 0) {
        // No data left to dequeue, return now.
        return ret;
    }

    while (head_bytes > 0) {
        if (samples_to_dequeue <= 0)
            break;

        intent_engine_sample_push((asr_sample_t *)ring_buf.get_ptr, appconfAUDIO_PIPELINE_FRAME_ADVANCE);
        ring_buf.get_ptr += (appconfAUDIO_PIPELINE_FRAME_ADVANCE * sizeof(asr_sample_t));
        head_bytes -= (appconfAUDIO_PIPELINE_FRAME_ADVANCE * sizeof(asr_sample_t));
        samples_to_dequeue -= appconfAUDIO_PIPELINE_FRAME_ADVANCE;
        ring_buf.count -= appconfAUDIO_PIPELINE_FRAME_ADVANCE;
    }

    return ret;
}

#endif // LOW_POWER_AUDIO_BUFFER_ENABLED
