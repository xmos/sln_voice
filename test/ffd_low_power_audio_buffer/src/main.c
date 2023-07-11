// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

/* App headers */
#include "app_conf.h"
#include "asr.h"
#include "low_power_audio_buffer.h"

#define XSTR(s)                     STR(s)
#define STR(x)                      #x

#define TEST_PRINTF(fmt, ...)       printf((fmt), ##__VA_ARGS__)

#define TEST_CASE_PRINTF(fmt, ...)  TEST_PRINTF("* %s" fmt "\n", __FUNCTION__, ##__VA_ARGS__)

#define TEST_ASSERT_INTS_ARE_EQUAL(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            printf("  - FAIL (Line: %d): " XSTR(actual) "\n", __LINE__); \
            printf("    Actual:   %d\n", (actual)); \
            printf("    Expected: %d\n", (expected)); \
            error_count++; \
        } \
    } while(0)

#define TEST_ASSERT_LONGS_ARE_EQUAL(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            printf("  - FAIL (Line: %d): " XSTR(actual) "\n", __LINE__); \
            printf("    Actual:   %ld\n", (actual)); \
            printf("    Expected: %ld\n", (expected)); \
            error_count++; \
        } \
    } while(0)

#define TEST_ASSERT_PTRS_ARE_EQUAL(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            printf(" - FAIL (Line: %d): " XSTR(actual) "\n", __LINE__); \
            printf("   Actual:   %p\n", (actual)); \
            printf("   Expected: %p\n", (expected)); \
            error_count++; \
        } \
    } while(0)

#define TOTAL_SAMPLES       (appconfAUDIO_PIPELINE_BUFFER_NUM_FRAMES * appconfAUDIO_PIPELINE_FRAME_ADVANCE)
#define LAST_SAMPLE_INDEX   (TOTAL_SAMPLES - 1)

#ifndef ring_buffer_t
typedef struct ring_buffer
{
    asr_sample_t * const buf;
    const uint32_t size;
    char *set_ptr;
    char *get_ptr;
    uint32_t count;
    uint8_t full;
    uint8_t empty;
} ring_buffer_t;
#endif

/* Internal buffers/structs from unit under test */
extern asr_sample_t sample_buf[];
extern ring_buffer_t ring_buf;

static uint32_t error_count = 0;
static asr_sample_t samples[appconfAUDIO_PIPELINE_FRAME_ADVANCE];

static uint8_t intent_engine_sample_push_skip_verify;
static asr_sample_t *intent_engine_sample_push_expected_buf;
static size_t intent_engine_sample_push_total_frames;
static uint32_t intent_engine_sample_push_error_count;

void setup_intent_engine_sample_push(char *expected_start_address)
{
    // Catch potential issues in test logic.
    assert((uint32_t)expected_start_address % sizeof(asr_sample_t) == 0);

    intent_engine_sample_push_expected_buf = (asr_sample_t *)expected_start_address;
    intent_engine_sample_push_error_count = error_count;
    intent_engine_sample_push_skip_verify = 1;
    intent_engine_sample_push_total_frames = 0;
}

void verify_intent_engine_sample_push_args(asr_sample_t *buf, size_t frames)
{
    const uint32_t tail_addr = ((uint32_t)ring_buf.buf + ring_buf.size);

    if (intent_engine_sample_push_skip_verify)
        return;

    TEST_ASSERT_PTRS_ARE_EQUAL(intent_engine_sample_push_expected_buf, buf);
    TEST_ASSERT_LONGS_ARE_EQUAL((uint32_t)appconfAUDIO_PIPELINE_FRAME_ADVANCE, (uint32_t)frames);
    intent_engine_sample_push_expected_buf += frames;

    if ((uint32_t)intent_engine_sample_push_expected_buf >= tail_addr)
        intent_engine_sample_push_expected_buf = ring_buf.buf;

    if (intent_engine_sample_push_error_count != error_count)
        intent_engine_sample_push_skip_verify = 0;
}

void verify_sample_buffer_state(uint32_t *starting_index,
                                uint32_t starting_value,
                                long num_samples)
{
    uint32_t error_count_last = error_count;

    for (long i = 0; i < num_samples; i++) {
        TEST_ASSERT_INTS_ARE_EQUAL(starting_value, sample_buf[(*starting_index)]);

        if (error_count != error_count_last) {
            printf("    Index:    %ld\n", *starting_index);
            break;
        }

        starting_value++;
        if (++(*starting_index) >= TOTAL_SAMPLES)
            (*starting_index) = 0;
    }
}

void fill_frames(size_t count, uint32_t starting_sample_value)
{
    memset(samples, 0xFF, sizeof(samples));

    for (size_t i = 0; i < count; i++) {
        samples[i] = starting_sample_value;
        starting_sample_value++;
    }
}

void init_sample_buffer(void)
{
    // Set each sample value to its index. This helps with detecting
    // modification and interactions with the sample buffer.
    for (size_t i = 0; i < ring_buf.size / sizeof(asr_sample_t); i++) {
        sample_buf[i] = i;
    }
}

void set_ring_buffer_state(char *buffer_set_ptr,
                           char *buffer_get_ptr,
                           uint32_t buffer_count,
                           uint8_t buffer_full,
                           uint8_t buffer_empty)
{
    ring_buf.set_ptr = buffer_set_ptr;
    ring_buf.get_ptr = buffer_get_ptr;
    ring_buf.count = buffer_count;
    ring_buf.full = buffer_full;
    ring_buf.empty = buffer_empty;
}

void reset_ring_buffer_state(void)
{
    set_ring_buffer_state((char *)sample_buf, (char *)sample_buf, 0, 0, 1);
}

void verify_initial_buffer_state(void)
{
    uint32_t expected_buf_size = (uint32_t)(TOTAL_SAMPLES * sizeof(asr_sample_t));
    uint8_t expected_full_state = 0;
    uint8_t expected_empty_state = 1;
    uint32_t expected_frame_count = 0;
    char *expected_set_ptr = (char *)sample_buf;
    char *expected_get_ptr = (char *)sample_buf;

    TEST_CASE_PRINTF();
    TEST_ASSERT_INTS_ARE_EQUAL(expected_full_state, ring_buf.full);
    TEST_ASSERT_INTS_ARE_EQUAL(expected_empty_state, ring_buf.empty);
    TEST_ASSERT_LONGS_ARE_EQUAL(expected_buf_size, ring_buf.size);
    TEST_ASSERT_LONGS_ARE_EQUAL(expected_frame_count, ring_buf.count);
    TEST_ASSERT_PTRS_ARE_EQUAL(expected_set_ptr, ring_buf.set_ptr);
    TEST_ASSERT_PTRS_ARE_EQUAL(expected_get_ptr, ring_buf.get_ptr);
}

void verify_set_pointer_wraps_around(void)
{
    const uint32_t starting_sample_value = TOTAL_SAMPLES;
    const uint32_t sample_count = 0;
    uint32_t samples_to_enqueue = 1;
    uint8_t expected_full_state = 0;
    uint8_t expected_empty_state = 0;
    uint32_t expected_frame_count = samples_to_enqueue;
    char *expected_set_ptr = (char *)(sample_buf);
    char *expected_get_ptr = (char *)(sample_buf);
    uint32_t starting_sample_index_to_verify = 0;

    TEST_CASE_PRINTF();
    init_sample_buffer(); // Reinitialize to decouple test cases.
    // Set the buffer to the tail of the buffer.
    set_ring_buffer_state((char *)(sample_buf + LAST_SAMPLE_INDEX),
                          expected_get_ptr,
                          sample_count,
                          expected_full_state,
                          1);
    fill_frames(samples_to_enqueue, starting_sample_value);

    low_power_audio_buffer_enqueue(samples, samples_to_enqueue);

    TEST_ASSERT_INTS_ARE_EQUAL(expected_full_state, ring_buf.full);
    TEST_ASSERT_INTS_ARE_EQUAL(expected_empty_state, ring_buf.empty);
    TEST_ASSERT_LONGS_ARE_EQUAL(expected_frame_count, ring_buf.count);
    TEST_ASSERT_PTRS_ARE_EQUAL(expected_set_ptr, ring_buf.set_ptr);
    TEST_ASSERT_PTRS_ARE_EQUAL(expected_get_ptr, ring_buf.get_ptr);

    // Verify that only the samples enqueued have been modified.
    verify_sample_buffer_state(&starting_sample_index_to_verify, 0, TOTAL_SAMPLES - 1);
    verify_sample_buffer_state(&starting_sample_index_to_verify, starting_sample_value, samples_to_enqueue);
}

void verify_get_pointer_wraps_around(void)
{
    uint32_t max_dequeue_frames = appconfAUDIO_PIPELINE_BUFFER_NUM_FRAMES;
    uint32_t sample_count = appconfAUDIO_PIPELINE_FRAME_ADVANCE;
    uint8_t expected_full_state = 0;
    uint8_t expected_empty_state = 1;
    uint32_t expected_frame_count = 0;
    uint32_t expected_frames_dequeued = sample_count;
    char *expected_set_ptr = (char *)(sample_buf);
    char *expected_get_ptr = (char *)(sample_buf);
    char *expected_get_ptr_start = (char *)(sample_buf + LAST_SAMPLE_INDEX);
    uint32_t starting_sample_index_to_verify = 0;

    TEST_CASE_PRINTF();
    init_sample_buffer(); // Reinitialize to decouple test cases.
    setup_intent_engine_sample_push(expected_get_ptr_start);
    // Set the buffer to the tail of the buffer.
    set_ring_buffer_state(expected_set_ptr,
                          expected_get_ptr_start,
                          sample_count,
                          expected_full_state,
                          0);

    uint32_t frames_dequeued = low_power_audio_buffer_dequeue(max_dequeue_frames);

    TEST_ASSERT_INTS_ARE_EQUAL(expected_full_state, ring_buf.full);
    TEST_ASSERT_INTS_ARE_EQUAL(expected_empty_state, ring_buf.empty);
    TEST_ASSERT_LONGS_ARE_EQUAL(expected_frame_count, ring_buf.count);
    TEST_ASSERT_LONGS_ARE_EQUAL(expected_frames_dequeued, frames_dequeued);
    TEST_ASSERT_PTRS_ARE_EQUAL(expected_set_ptr, ring_buf.set_ptr);
    TEST_ASSERT_PTRS_ARE_EQUAL(expected_get_ptr, ring_buf.get_ptr);

    // No samples in buffer should have been modified.
    verify_sample_buffer_state(&starting_sample_index_to_verify, 0, TOTAL_SAMPLES);
}

void verify_enqueuing_samples_less_than_buffer_capacity_remains_not_full(uint32_t samples_to_enqueue)
{
    uint32_t starting_sample_value = TOTAL_SAMPLES;
    uint8_t expected_full_state = 0;
    uint8_t expected_empty_state = 0;
    uint32_t expected_frame_count = samples_to_enqueue;
    char *expected_set_ptr = (char *)(sample_buf + samples_to_enqueue);
    char *expected_get_ptr = (char *)(sample_buf);
    uint32_t starting_sample_index_to_verify = 0;
    uint32_t sample_value = starting_sample_value;

    TEST_CASE_PRINTF(": Enqueuing %ld sample(s).", samples_to_enqueue);
    init_sample_buffer(); // Reinitialize to decouple test cases.
    reset_ring_buffer_state();

    while (samples_to_enqueue > 0) {
        uint32_t enqueue_samples = (samples_to_enqueue > appconfAUDIO_PIPELINE_FRAME_ADVANCE) ?
            appconfAUDIO_PIPELINE_FRAME_ADVANCE :
            samples_to_enqueue;

        fill_frames(enqueue_samples, sample_value);
        low_power_audio_buffer_enqueue(samples, enqueue_samples);
        samples_to_enqueue -= enqueue_samples;
        sample_value += enqueue_samples;
    }

    TEST_ASSERT_INTS_ARE_EQUAL(expected_full_state, ring_buf.full);
    TEST_ASSERT_INTS_ARE_EQUAL(expected_empty_state, ring_buf.empty);
    TEST_ASSERT_LONGS_ARE_EQUAL(expected_frame_count, ring_buf.count);
    TEST_ASSERT_PTRS_ARE_EQUAL(expected_set_ptr, ring_buf.set_ptr);
    TEST_ASSERT_PTRS_ARE_EQUAL(expected_get_ptr, ring_buf.get_ptr);

    // Verify that only the samples enqueued have been modified.
    verify_sample_buffer_state(&starting_sample_index_to_verify, starting_sample_value, expected_frame_count);
    verify_sample_buffer_state(&starting_sample_index_to_verify, starting_sample_index_to_verify, TOTAL_SAMPLES - expected_frame_count);
}

void verify_enqueueing_samples_equal_to_buffer_capacity_reports_full(uint32_t frame_index)
{
    const uint32_t init_buf_count = 0;
    const uint32_t init_buf_full_state = 0;
    const uint32_t init_buf_empty_state = 1;
    const uint32_t starting_sample_value = TOTAL_SAMPLES;
    uint32_t samples_to_enqueue = TOTAL_SAMPLES;
    uint8_t expected_full_state = 1;
    uint8_t expected_empty_state = 0;
    uint32_t expected_frame_count = samples_to_enqueue;
    char *expected_set_ptr = (char *)(sample_buf + frame_index);
    char *expected_get_ptr = (char *)(sample_buf + frame_index); // When full, the get pointer moves with the set pointer.
    uint32_t starting_sample_index_to_verify = frame_index;
    uint32_t sample_value = starting_sample_value;

    TEST_CASE_PRINTF(": Enqueuing %ld sample(s) at sample index %ld.",
                     samples_to_enqueue, frame_index);
    init_sample_buffer(); // Reinitialize to decouple test cases.
    set_ring_buffer_state(expected_set_ptr,
                          expected_get_ptr,
                          init_buf_count,
                          init_buf_full_state,
                          init_buf_empty_state);

    while (samples_to_enqueue > 0) {
        uint32_t enqueue_samples = (samples_to_enqueue > appconfAUDIO_PIPELINE_FRAME_ADVANCE) ?
            appconfAUDIO_PIPELINE_FRAME_ADVANCE :
            samples_to_enqueue;

        fill_frames(enqueue_samples, sample_value);
        low_power_audio_buffer_enqueue(samples, enqueue_samples);
        samples_to_enqueue -= enqueue_samples;
        sample_value += enqueue_samples;
    }

    TEST_ASSERT_INTS_ARE_EQUAL(expected_full_state, ring_buf.full);
    TEST_ASSERT_INTS_ARE_EQUAL(expected_empty_state, ring_buf.empty);
    TEST_ASSERT_LONGS_ARE_EQUAL(expected_frame_count, ring_buf.count);
    TEST_ASSERT_PTRS_ARE_EQUAL(expected_set_ptr, ring_buf.set_ptr);
    TEST_ASSERT_PTRS_ARE_EQUAL(expected_get_ptr, ring_buf.get_ptr);

    // Verify that the samples enqueued have been modified.
    verify_sample_buffer_state(&starting_sample_index_to_verify, starting_sample_value, expected_frame_count);
}

void verify_enqueue_samples_greater_than_buf_capacity_overwrites_oldest(uint32_t frame_index)
{
    const uint32_t init_buf_count = 0;
    const uint32_t init_buf_full_state = 0;
    const uint32_t init_buf_empty_state = 1;
    const uint32_t starting_sample_value = TOTAL_SAMPLES;
    char *init_set_ptr = (char *)(sample_buf + frame_index);
    char *init_get_ptr = (char *)(sample_buf + frame_index);
    uint32_t samples_to_enqueue = TOTAL_SAMPLES + 1;
    uint8_t expected_full_state = 1;
    uint8_t expected_empty_state = 0;
    uint32_t expected_frame_count = TOTAL_SAMPLES;
    char *expected_set_ptr = (char *)(sample_buf + ((frame_index + 1) % TOTAL_SAMPLES));
    char *expected_get_ptr = (char *)(sample_buf + ((frame_index + 1) % TOTAL_SAMPLES)); // When full, the get pointer moves with the set pointer.
    uint32_t starting_sample_index_to_verify = (frame_index + 1) % TOTAL_SAMPLES;
    uint32_t sample_value = starting_sample_value;

    TEST_CASE_PRINTF(": Enqueuing %ld sample(s) at sample index %ld.",
                     samples_to_enqueue, frame_index);
    init_sample_buffer(); // Reinitialize to decouple test cases.
    set_ring_buffer_state(init_set_ptr,
                          init_get_ptr,
                          init_buf_count,
                          init_buf_full_state,
                          init_buf_empty_state);

    while (samples_to_enqueue > 0) {
        uint32_t enqueue_samples = (samples_to_enqueue > appconfAUDIO_PIPELINE_FRAME_ADVANCE) ?
            appconfAUDIO_PIPELINE_FRAME_ADVANCE :
            samples_to_enqueue;

        fill_frames(enqueue_samples, sample_value);
        low_power_audio_buffer_enqueue(samples, enqueue_samples);
        samples_to_enqueue -= enqueue_samples;
        sample_value += enqueue_samples;
    }

    TEST_ASSERT_INTS_ARE_EQUAL(expected_full_state, ring_buf.full);
    TEST_ASSERT_INTS_ARE_EQUAL(expected_empty_state, ring_buf.empty);
    TEST_ASSERT_LONGS_ARE_EQUAL(expected_frame_count, ring_buf.count);
    TEST_ASSERT_PTRS_ARE_EQUAL(expected_set_ptr, ring_buf.set_ptr);
    TEST_ASSERT_PTRS_ARE_EQUAL(expected_get_ptr, ring_buf.get_ptr);

    /* Verify the enqueued samples. The first sample enqueued should have been
     * overwritten, meaning it has the largest value in the sequence and should
     * be the last value to verify. */
    verify_sample_buffer_state(&starting_sample_index_to_verify, starting_sample_value + 1, TOTAL_SAMPLES);
}

void verify_dequeuing_empty_buffer_does_not_output_samples(void)
{
    uint32_t max_dequeue_frames = 1;
    uint8_t expected_full_state = 0;
    uint8_t expected_empty_state = 1;
    uint32_t expected_frames_dequeued = 0;
    uint32_t expected_frame_count = 0;
    char *expected_set_ptr = (char *)sample_buf;
    char *expected_get_ptr = (char *)sample_buf;
    char *expected_get_ptr_start = (char *)sample_buf;
    uint32_t starting_sample_index_to_verify = 0;

    TEST_CASE_PRINTF();
    init_sample_buffer(); // Reinitialize to decouple test cases.
    setup_intent_engine_sample_push(expected_get_ptr_start);
    reset_ring_buffer_state();

    uint32_t frames_dequeued = low_power_audio_buffer_dequeue(max_dequeue_frames);

    TEST_ASSERT_INTS_ARE_EQUAL(expected_full_state, ring_buf.full);
    TEST_ASSERT_INTS_ARE_EQUAL(expected_empty_state, ring_buf.empty);
    TEST_ASSERT_LONGS_ARE_EQUAL(expected_frames_dequeued, frames_dequeued);
    TEST_ASSERT_LONGS_ARE_EQUAL(expected_frame_count, ring_buf.count);
    TEST_ASSERT_PTRS_ARE_EQUAL(expected_set_ptr, ring_buf.set_ptr);
    TEST_ASSERT_PTRS_ARE_EQUAL(expected_get_ptr, ring_buf.get_ptr);

    // No samples in buffer should have been modified.
    verify_sample_buffer_state(&starting_sample_index_to_verify, 0, TOTAL_SAMPLES);
}

void verify_dequeuing_non_full_frame_is_not_possible(uint32_t samples_to_enqueue)
{
    const uint32_t max_dequeue_frames = appconfAUDIO_PIPELINE_BUFFER_NUM_FRAMES;
    const uint8_t expected_full_state = 0;
    const uint8_t expected_empty_state = 0;
    const uint32_t expected_frames_dequeued = 0;
    uint32_t expected_frame_count = samples_to_enqueue;
    char *expected_set_ptr = (char *)(sample_buf + samples_to_enqueue);
    char *expected_get_ptr = (char *)sample_buf;
    char *expected_get_ptr_start = (char *)sample_buf;
    uint32_t starting_sample_index_to_verify = 0;

    TEST_CASE_PRINTF(": Dequeuing %ld enqueued sample(s).", samples_to_enqueue);
    init_sample_buffer(); // Reinitialize to decouple test cases.
    setup_intent_engine_sample_push(expected_get_ptr_start);
    set_ring_buffer_state(expected_set_ptr,
                          expected_get_ptr,
                          expected_frame_count,
                          expected_full_state,
                          expected_empty_state);

    uint32_t frames_dequeued = low_power_audio_buffer_dequeue(max_dequeue_frames);

    TEST_ASSERT_INTS_ARE_EQUAL(expected_full_state, ring_buf.full);
    TEST_ASSERT_INTS_ARE_EQUAL(expected_empty_state, ring_buf.empty);
    TEST_ASSERT_LONGS_ARE_EQUAL(expected_frame_count, ring_buf.count);
    TEST_ASSERT_LONGS_ARE_EQUAL(expected_frames_dequeued, frames_dequeued);
    TEST_ASSERT_PTRS_ARE_EQUAL(expected_set_ptr, ring_buf.set_ptr);
    TEST_ASSERT_PTRS_ARE_EQUAL(expected_get_ptr, ring_buf.get_ptr);

    // No samples in buffer should have been modified.
    verify_sample_buffer_state(&starting_sample_index_to_verify, 0, TOTAL_SAMPLES);
}

void verify_dequeuing_partially_reports_not_empty(uint32_t frame_index)
{
    const uint32_t init_buf_full_state = 1;
    const uint32_t init_buf_empty_state = 0;
    const uint32_t init_buf_count = TOTAL_SAMPLES;
    uint32_t max_dequeue_frames = appconfAUDIO_PIPELINE_BUFFER_NUM_FRAMES - 1;
    uint8_t expected_full_state = 0;
    uint8_t expected_empty_state = 0;
    uint32_t expected_frame_count = appconfAUDIO_PIPELINE_FRAME_ADVANCE;
    uint32_t expected_frames_dequeued = max_dequeue_frames * appconfAUDIO_PIPELINE_FRAME_ADVANCE;
    char *expected_set_ptr = (char *)(sample_buf + frame_index);
    char *expected_get_ptr = (char *)(sample_buf + (
        (frame_index + max_dequeue_frames * appconfAUDIO_PIPELINE_FRAME_ADVANCE) % TOTAL_SAMPLES));
    char *expected_get_ptr_start = (char *)(sample_buf + frame_index);
    uint32_t starting_sample_index_to_verify = 0;

    TEST_CASE_PRINTF(": Dequeuing %ld of the %d enqueued frames at sample index %ld.",
                     max_dequeue_frames,
                     appconfAUDIO_PIPELINE_BUFFER_NUM_FRAMES,
                     frame_index);
    init_sample_buffer(); // Reinitialize to decouple test cases.
    setup_intent_engine_sample_push(expected_get_ptr_start);
    set_ring_buffer_state(expected_set_ptr,
                          expected_get_ptr_start,
                          init_buf_count,
                          init_buf_full_state,
                          init_buf_empty_state);

    uint32_t frames_dequeued = low_power_audio_buffer_dequeue(max_dequeue_frames);

    TEST_ASSERT_INTS_ARE_EQUAL(expected_full_state, ring_buf.full);
    TEST_ASSERT_INTS_ARE_EQUAL(expected_empty_state, ring_buf.empty);
    TEST_ASSERT_LONGS_ARE_EQUAL(expected_frame_count, ring_buf.count);
    TEST_ASSERT_LONGS_ARE_EQUAL(expected_frames_dequeued, frames_dequeued);
    TEST_ASSERT_PTRS_ARE_EQUAL(expected_set_ptr, ring_buf.set_ptr);
    TEST_ASSERT_PTRS_ARE_EQUAL(expected_get_ptr, ring_buf.get_ptr);

    // No samples in buffer should have been modified.
    verify_sample_buffer_state(&starting_sample_index_to_verify, 0, TOTAL_SAMPLES);
}

void verify_dequeuing_all_frames_reports_empty(uint32_t frame_index)
{
    const uint32_t init_buf_full_state = 1;
    const uint32_t init_buf_empty_state = 0;
    const uint32_t init_buf_count = TOTAL_SAMPLES;
    uint32_t max_dequeue_frames = appconfAUDIO_PIPELINE_BUFFER_NUM_FRAMES;
    uint8_t expected_full_state = 0;
    uint8_t expected_empty_state = 1;
    uint32_t expected_frame_count = 0;
    uint32_t expected_frames_dequeued = max_dequeue_frames * appconfAUDIO_PIPELINE_FRAME_ADVANCE;
    char *expected_set_ptr = (char *)(sample_buf + frame_index);
    char *expected_get_ptr = (char *)(sample_buf + (
        (frame_index + max_dequeue_frames * appconfAUDIO_PIPELINE_FRAME_ADVANCE) %
        (TOTAL_SAMPLES)));
    char *expected_get_ptr_start = (char *)(sample_buf + frame_index);
    uint32_t starting_sample_index_to_verify = 0;

    TEST_CASE_PRINTF(": Dequeuing all %ld enqueued frames at sample index %ld.",
                     max_dequeue_frames,
                     frame_index);
    init_sample_buffer(); // Reinitialize to decouple test cases.
    setup_intent_engine_sample_push(expected_get_ptr_start);
    set_ring_buffer_state(expected_set_ptr,
                          expected_get_ptr_start,
                          init_buf_count,
                          init_buf_full_state,
                          init_buf_empty_state);

    uint32_t frames_dequeued = low_power_audio_buffer_dequeue(max_dequeue_frames);

    TEST_ASSERT_INTS_ARE_EQUAL(expected_full_state, ring_buf.full);
    TEST_ASSERT_INTS_ARE_EQUAL(expected_empty_state, ring_buf.empty);
    TEST_ASSERT_LONGS_ARE_EQUAL(expected_frame_count, ring_buf.count);
    TEST_ASSERT_LONGS_ARE_EQUAL(expected_frames_dequeued, frames_dequeued);
    TEST_ASSERT_PTRS_ARE_EQUAL(expected_set_ptr, ring_buf.set_ptr);
    TEST_ASSERT_PTRS_ARE_EQUAL(expected_get_ptr, ring_buf.get_ptr);

    // No samples in buffer should have been modified.
    verify_sample_buffer_state(&starting_sample_index_to_verify, 0, TOTAL_SAMPLES);
}

int main(void)
{
    TEST_PRINTF("CONFIGURATION:\n");
    TEST_PRINTF("- Frame Size (Samples): %d\n", appconfAUDIO_PIPELINE_FRAME_ADVANCE);
    TEST_PRINTF("- Buffer Size (Frames): %d\n", appconfAUDIO_PIPELINE_BUFFER_NUM_FRAMES);
    TEST_PRINTF("- Sample Buffer Address: %p\n\n", sample_buf);

    /*
     * The initial state of the ring buffer should be fully known and match
     * expectations.
     */
    verify_initial_buffer_state();

    /*
     * The set/get pointers are to wrap around when they reach the tail of the
     * internal buffer.
     */
    verify_set_pointer_wraps_around();
    verify_get_pointer_wraps_around();

    /*
     * Enqueuing at least one sample results in the buffer no longer being empty.
     * The buffer remains not full when enqueuing samples less than the buffer's
     * capacity.
     */
    verify_enqueuing_samples_less_than_buffer_capacity_remains_not_full(1);
    verify_enqueuing_samples_less_than_buffer_capacity_remains_not_full(appconfAUDIO_PIPELINE_FRAME_ADVANCE);
    verify_enqueuing_samples_less_than_buffer_capacity_remains_not_full(appconfAUDIO_PIPELINE_FRAME_ADVANCE * appconfAUDIO_PIPELINE_BUFFER_NUM_FRAMES - 1);

    /*
     * Enqueuing samples to match the capacity of the buffer should result in
     * the buffer asserting the full flag; this behavior should be not be
     * impacted by the initial offset where enqueuing begins.
     */
    verify_enqueueing_samples_equal_to_buffer_capacity_reports_full(0);
    verify_enqueueing_samples_equal_to_buffer_capacity_reports_full(1);
    verify_enqueueing_samples_equal_to_buffer_capacity_reports_full(appconfAUDIO_PIPELINE_FRAME_ADVANCE);
    verify_enqueueing_samples_equal_to_buffer_capacity_reports_full(appconfAUDIO_PIPELINE_FRAME_ADVANCE * appconfAUDIO_PIPELINE_BUFFER_NUM_FRAMES - 2);
    verify_enqueueing_samples_equal_to_buffer_capacity_reports_full(appconfAUDIO_PIPELINE_FRAME_ADVANCE * appconfAUDIO_PIPELINE_BUFFER_NUM_FRAMES - 1);

    /*
     * The oldest samples in the queue are lost when ring buffer is full and new
     * data is written. Additionally, the get_ptr should follow the set_ptr
     * while in the full-state.
     */
    verify_enqueue_samples_greater_than_buf_capacity_overwrites_oldest(0);
    verify_enqueue_samples_greater_than_buf_capacity_overwrites_oldest(1);
    verify_enqueue_samples_greater_than_buf_capacity_overwrites_oldest(appconfAUDIO_PIPELINE_FRAME_ADVANCE * appconfAUDIO_PIPELINE_BUFFER_NUM_FRAMES - 1);

    /*
     * If the buffer is empty, no sample data should be released/reported by
     * low_power_audio_buffer_dequeue().
     */
    verify_dequeuing_empty_buffer_does_not_output_samples();

    /*
     * Data cannot be dequeued until at least appconfAUDIO_PIPELINE_FRAME_ADVANCE
     * samples are enqueued.
     */
    verify_dequeuing_non_full_frame_is_not_possible(1);
    verify_dequeuing_non_full_frame_is_not_possible(appconfAUDIO_PIPELINE_FRAME_ADVANCE - 1);

    /*
     * Dequeuing all but the last frame should deassert the full flag, the
     * empty flag should not assert.
     */
    verify_dequeuing_partially_reports_not_empty(0);
    verify_dequeuing_partially_reports_not_empty(appconfAUDIO_PIPELINE_FRAME_ADVANCE);
    verify_dequeuing_partially_reports_not_empty(appconfAUDIO_PIPELINE_FRAME_ADVANCE * (appconfAUDIO_PIPELINE_BUFFER_NUM_FRAMES - 2));
    verify_dequeuing_partially_reports_not_empty(appconfAUDIO_PIPELINE_FRAME_ADVANCE * (appconfAUDIO_PIPELINE_BUFFER_NUM_FRAMES - 1));

    /*
     * Dequeuing all frames should deassert the full flag and the empty flag
     * should assert.
     */
    verify_dequeuing_all_frames_reports_empty(0);
    verify_dequeuing_all_frames_reports_empty(appconfAUDIO_PIPELINE_FRAME_ADVANCE);
    verify_dequeuing_all_frames_reports_empty(appconfAUDIO_PIPELINE_FRAME_ADVANCE * (appconfAUDIO_PIPELINE_BUFFER_NUM_FRAMES - 2));
    verify_dequeuing_all_frames_reports_empty(appconfAUDIO_PIPELINE_FRAME_ADVANCE * (appconfAUDIO_PIPELINE_BUFFER_NUM_FRAMES - 1));

    if (error_count == 0) {
        TEST_PRINTF("\nTEST: PASS\n");
    } else {
        TEST_PRINTF("\nTEST: FAILED (Error Count = %ld)\n", error_count);
    }

    return 0;
}
