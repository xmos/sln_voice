// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* App headers */
#include "app_conf.h"
#include "low_power_audio_buffer.h"

#define XSTR(s)                     STR(s)
#define STR(x)                      #x

#define TEST_PRINTF(fmt, ...)       printf((fmt), ##__VA_ARGS__)

#define TEST_CASE_PRINTF(fmt, ...)  TEST_PRINTF("* " fmt "\n", ##__VA_ARGS__)

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

// TODO: temp for debugging
#define TEST_PRINT_PTR_FRAME_INDEX(p) \
    printf("    FRAME INDEX: %ld\n", (((uint32_t)p - (uint32_t)ring_buf.buf)/sizeof(int32_t)))

#define TEST_ASSERT_PTRS_ARE_EQUAL(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            printf(" - FAIL (Line: %d): " XSTR(actual) "\n", __LINE__); \
            printf("   Actual:   %p\n", (actual)); \
            TEST_PRINT_PTR_FRAME_INDEX(actual); \
            printf("   Expected: %p\n", (expected)); \
            TEST_PRINT_PTR_FRAME_INDEX(expected); \
            error_count++; \
        } \
    } while(0)

#ifndef ring_buffer_t
typedef struct ring_buffer
{
    int32_t * const buf;
    const uint32_t size;
    char *set_ptr;
    char *get_ptr;
    uint32_t count;
    uint8_t full;
    uint8_t empty;
} ring_buffer_t;
#endif

extern int32_t sample_buf[];
extern ring_buffer_t ring_buf;

void fill_array(int32_t* samples, size_t count);
void print_buffer_info(void);
void drive_buffer(int32_t **frames, uint32_t *frames_dequeued);

static uint32_t error_count = 0;
static int32_t frames[1][appconfAUDIO_PIPELINE_FRAME_ADVANCE];

void print_buffer_info(void)
{
    if (ring_buf.full)
    {
        printf("CNT = %3ld; SET_ELEM = %3ld; GET_ELEM = %3ld [[FULL]]\n\n",
            ring_buf.count,
            ((uint32_t)ring_buf.set_ptr - (uint32_t)ring_buf.buf) / sizeof(int32_t),
            ((uint32_t)ring_buf.get_ptr - (uint32_t)ring_buf.buf) / sizeof(int32_t));
    }
    else if (ring_buf.empty)
    {
        printf("CNT = %3ld; SET_ELEM = %3ld; GET_ELEM = %3ld [[EMPTY]]\n\n",
            ring_buf.count,
            ((uint32_t)ring_buf.set_ptr - (uint32_t)ring_buf.buf) / sizeof(int32_t),
            ((uint32_t)ring_buf.get_ptr - (uint32_t)ring_buf.buf) / sizeof(int32_t));
    }
    else {
        printf("CNT = %3ld; SET_ELEM = %3ld; GET_ELEM = %3ld\n\n",
            ring_buf.count,
            ((uint32_t)ring_buf.set_ptr - (uint32_t)ring_buf.buf) / sizeof(int32_t),
            ((uint32_t)ring_buf.get_ptr - (uint32_t)ring_buf.buf) / sizeof(int32_t));
    }
}

void fill_frames(int32_t *frames, size_t count, uint32_t starting_frame_value)
{
    for (size_t i = 0; i < count; i++) {
        frames[i] = starting_frame_value;
        starting_frame_value++;
    }
}

void init_buffer(void)
{
    for (size_t i = 0; i < ring_buf.size / sizeof(int32_t); i++) {
        sample_buf[i] = i;
    }
}

void set_buffer_state(char *buffer_set_ptr,
                      char *buffer_get_ptr,
                      uint32_t buffer_frame_count,
                      uint8_t buffer_full,
                      uint8_t buffer_empty)
{
    ring_buf.set_ptr = buffer_set_ptr;
    ring_buf.get_ptr = buffer_get_ptr;
    ring_buf.count = buffer_frame_count;
    ring_buf.full = buffer_full;
    ring_buf.empty = buffer_empty;
}

void reset_buffer_state(void)
{
    set_buffer_state((char *)sample_buf, (char *)sample_buf, 0, 0, 1);
}

void verify_initial_buffer_state(void)
{
    uint32_t expected_buf_size =
        (uint32_t)(appconfAUDIO_PIPELINE_BUFFER_NUM_PACKETS *
        appconfAUDIO_PIPELINE_FRAME_ADVANCE * sizeof(uint32_t));
    uint8_t expected_full_state = 0;
    uint8_t expected_empty_state = 1;
    uint32_t expected_frame_count = 0;
    char *expected_set_ptr = (char *)sample_buf;
    char *expected_get_ptr = (char *)sample_buf;

    TEST_CASE_PRINTF("Verify initial buffer state is correct.");
    TEST_ASSERT_INTS_ARE_EQUAL(expected_full_state, ring_buf.full);
    TEST_ASSERT_INTS_ARE_EQUAL(expected_empty_state, ring_buf.empty);
    TEST_ASSERT_LONGS_ARE_EQUAL(expected_buf_size, ring_buf.size);
    TEST_ASSERT_LONGS_ARE_EQUAL(expected_frame_count, ring_buf.count);
    TEST_ASSERT_PTRS_ARE_EQUAL(expected_set_ptr, ring_buf.set_ptr);
    TEST_ASSERT_PTRS_ARE_EQUAL(expected_get_ptr, ring_buf.get_ptr);

}

void verify_set_pointer_wraps_around(void)
{
    const uint32_t starting_frame_value = (appconfAUDIO_PIPELINE_BUFFER_NUM_PACKETS * appconfAUDIO_PIPELINE_FRAME_ADVANCE);
    const uint32_t frame_count = 0;
    uint32_t frames_to_enqueue = 1;
    uint8_t expected_full_state = 0;
    uint8_t expected_empty_state = 0;
    uint32_t expected_frame_count = frames_to_enqueue;
    char *expected_set_ptr = (char *)(sample_buf);
    char *expected_get_ptr = (char *)(sample_buf);

    // Set the buffer to the tail of the buffer.
    set_buffer_state((char *)(sample_buf + (appconfAUDIO_PIPELINE_BUFFER_NUM_PACKETS * appconfAUDIO_PIPELINE_FRAME_ADVANCE) - 1),
                     expected_get_ptr,
                     frame_count,
                     expected_full_state,
                     1);
    memset(frames, 0xFF, sizeof(frames));
    fill_frames((int32_t *)frames, frames_to_enqueue, starting_frame_value);

    TEST_CASE_PRINTF("Verify the set pointer wraps around.");
    low_power_audio_buffer_enqueue((int32_t *)frames, frames_to_enqueue);

    TEST_ASSERT_INTS_ARE_EQUAL(expected_full_state, ring_buf.full);
    TEST_ASSERT_INTS_ARE_EQUAL(expected_empty_state, ring_buf.empty);
    TEST_ASSERT_LONGS_ARE_EQUAL(expected_frame_count, ring_buf.count);
    TEST_ASSERT_PTRS_ARE_EQUAL(expected_set_ptr, ring_buf.set_ptr);
    TEST_ASSERT_PTRS_ARE_EQUAL(expected_get_ptr, ring_buf.get_ptr);
}

void verify_get_pointer_wraps_around(void)
{
    uint32_t max_dequeue_packets = appconfAUDIO_PIPELINE_BUFFER_NUM_PACKETS;
    uint32_t frame_count = appconfAUDIO_PIPELINE_FRAME_ADVANCE;
    uint8_t expected_full_state = 0;
    uint8_t expected_empty_state = 1;
    uint32_t expected_frame_count = 0;
    uint32_t expected_frames_dequeued = frame_count;
    char *expected_set_ptr = (char *)(sample_buf);
    char *expected_get_ptr = (char *)(sample_buf);

    // Set the buffer to the tail of the buffer.
    set_buffer_state(expected_set_ptr,
                     (char *)(sample_buf + (appconfAUDIO_PIPELINE_BUFFER_NUM_PACKETS * appconfAUDIO_PIPELINE_FRAME_ADVANCE) - 1),
                     frame_count,
                     expected_full_state,
                     0);

    TEST_CASE_PRINTF("Verify the get pointer wraps around.");
    uint32_t frames_dequeued = low_power_audio_buffer_dequeue(max_dequeue_packets);

    TEST_ASSERT_INTS_ARE_EQUAL(expected_full_state, ring_buf.full);
    TEST_ASSERT_INTS_ARE_EQUAL(expected_empty_state, ring_buf.empty);
    TEST_ASSERT_LONGS_ARE_EQUAL(expected_frame_count, ring_buf.count);
    TEST_ASSERT_LONGS_ARE_EQUAL(expected_frames_dequeued, frames_dequeued);
    TEST_ASSERT_PTRS_ARE_EQUAL(expected_set_ptr, ring_buf.set_ptr);
    TEST_ASSERT_PTRS_ARE_EQUAL(expected_get_ptr, ring_buf.get_ptr);
}

void verify_enqueuing_frames_less_than_buffer_capacity_remains_not_full(uint32_t frames_to_enqueue)
{
    uint32_t starting_frame_value = (appconfAUDIO_PIPELINE_BUFFER_NUM_PACKETS * appconfAUDIO_PIPELINE_FRAME_ADVANCE);
    uint8_t expected_full_state = 0;
    uint8_t expected_empty_state = 0;
    uint32_t expected_frame_count = frames_to_enqueue;
    char *expected_set_ptr = (char *)(sample_buf + frames_to_enqueue);
    char *expected_get_ptr = (char *)(sample_buf);
    reset_buffer_state();

    TEST_CASE_PRINTF("Verify enqueuing %ld frame(s).", frames_to_enqueue);
    while (frames_to_enqueue > 0) {
        uint32_t enqueue_frames = (frames_to_enqueue > appconfAUDIO_PIPELINE_FRAME_ADVANCE) ?
            appconfAUDIO_PIPELINE_FRAME_ADVANCE :
            frames_to_enqueue;

        memset(frames, 0xFF, sizeof(frames));
        fill_frames((int32_t *)frames, enqueue_frames, starting_frame_value);
        low_power_audio_buffer_enqueue((int32_t *)frames, enqueue_frames);
        frames_to_enqueue -= enqueue_frames;
    }

    TEST_ASSERT_INTS_ARE_EQUAL(expected_full_state, ring_buf.full);
    TEST_ASSERT_INTS_ARE_EQUAL(expected_empty_state, ring_buf.empty);
    TEST_ASSERT_LONGS_ARE_EQUAL(expected_frame_count, ring_buf.count);
    TEST_ASSERT_PTRS_ARE_EQUAL(expected_set_ptr, ring_buf.set_ptr);
    TEST_ASSERT_PTRS_ARE_EQUAL(expected_get_ptr, ring_buf.get_ptr);
}

void verify_enqueueing_frames_equal_to_buffer_capacity_reports_full(uint32_t frame_offset)
{
    const uint32_t init_buf_count = 0;
    const uint32_t init_buf_full_state = 0;
    const uint32_t init_buf_empty_state = 1;
    const uint32_t starting_frame_value = (appconfAUDIO_PIPELINE_BUFFER_NUM_PACKETS * appconfAUDIO_PIPELINE_FRAME_ADVANCE);
    uint32_t frames_to_enqueue = (appconfAUDIO_PIPELINE_FRAME_ADVANCE * appconfAUDIO_PIPELINE_BUFFER_NUM_PACKETS);
    uint8_t expected_full_state = 1;
    uint8_t expected_empty_state = 0;
    uint32_t expected_frame_count = frames_to_enqueue;
    char *expected_set_ptr = (char *)(sample_buf + frame_offset);
    char *expected_get_ptr = (char *)(sample_buf + frame_offset); // When full, the get pointer moves with the set pointer.
    set_buffer_state(expected_set_ptr,
                     expected_get_ptr,
                     init_buf_count,
                     init_buf_full_state,
                     init_buf_empty_state);

    TEST_CASE_PRINTF("Verify enqueuing %ld frame(s) at frame offset %ld.",
                     frames_to_enqueue, frame_offset);
    while (frames_to_enqueue > 0) {
        uint32_t enqueue_frames = (frames_to_enqueue > appconfAUDIO_PIPELINE_FRAME_ADVANCE) ?
            appconfAUDIO_PIPELINE_FRAME_ADVANCE :
            frames_to_enqueue;

        memset(frames, 0xFF, sizeof(frames));
        fill_frames((int32_t *)frames, enqueue_frames, starting_frame_value);
        low_power_audio_buffer_enqueue((int32_t *)frames, enqueue_frames);
        frames_to_enqueue -= enqueue_frames;
    }

    TEST_ASSERT_INTS_ARE_EQUAL(expected_full_state, ring_buf.full);
    TEST_ASSERT_INTS_ARE_EQUAL(expected_empty_state, ring_buf.empty);
    TEST_ASSERT_LONGS_ARE_EQUAL(expected_frame_count, ring_buf.count);
    TEST_ASSERT_PTRS_ARE_EQUAL(expected_set_ptr, ring_buf.set_ptr);
    TEST_ASSERT_PTRS_ARE_EQUAL(expected_get_ptr, ring_buf.get_ptr);
}

void verify_dequeuing_empty_buffer_does_not_output_frames(void)
{
    uint32_t max_dequeue_packets = 1;
    uint8_t expected_full_state = 0;
    uint8_t expected_empty_state = 1;
    uint32_t expected_frames_dequeued = 0;
    uint32_t expected_frame_count = 0;
    char *expected_set_ptr = (char *)sample_buf;
    char *expected_get_ptr = (char *)sample_buf;
    reset_buffer_state();

    TEST_CASE_PRINTF("Verify dequeuing an empty buffer does nothing.");
    uint32_t frames_dequeued = low_power_audio_buffer_dequeue(max_dequeue_packets);

    TEST_ASSERT_INTS_ARE_EQUAL(expected_full_state, ring_buf.full);
    TEST_ASSERT_INTS_ARE_EQUAL(expected_empty_state, ring_buf.empty);
    TEST_ASSERT_LONGS_ARE_EQUAL(expected_frames_dequeued, frames_dequeued);
    TEST_ASSERT_LONGS_ARE_EQUAL(expected_frame_count, ring_buf.count);
    TEST_ASSERT_PTRS_ARE_EQUAL(expected_set_ptr, ring_buf.set_ptr);
    TEST_ASSERT_PTRS_ARE_EQUAL(expected_get_ptr, ring_buf.get_ptr);
}

void verify_dequeuing_non_full_packet_is_not_possible(uint32_t frames_to_enqueue)
{
    const uint32_t starting_frame_value = (appconfAUDIO_PIPELINE_BUFFER_NUM_PACKETS * appconfAUDIO_PIPELINE_FRAME_ADVANCE);
    const uint32_t max_dequeue_packets = appconfAUDIO_PIPELINE_BUFFER_NUM_PACKETS;
    const uint8_t expected_full_state = 0;
    const uint8_t expected_empty_state = 0;
    const uint32_t expected_frames_dequeued = 0;
    uint32_t expected_frame_count = frames_to_enqueue;
    char *expected_set_ptr = (char *)(sample_buf + frames_to_enqueue);
    char *expected_get_ptr = (char *)(sample_buf);
    reset_buffer_state();
    memset(frames, 0xFF, sizeof(frames));
    fill_frames((int32_t *)frames, frames_to_enqueue, starting_frame_value);

    TEST_CASE_PRINTF("Verify dequeuing %ld enqueued frame(s).", frames_to_enqueue);
    low_power_audio_buffer_enqueue((int32_t *)frames, frames_to_enqueue);
    uint32_t frames_dequeued = low_power_audio_buffer_dequeue(max_dequeue_packets);

    TEST_ASSERT_INTS_ARE_EQUAL(expected_full_state, ring_buf.full);
    TEST_ASSERT_INTS_ARE_EQUAL(expected_empty_state, ring_buf.empty);
    TEST_ASSERT_LONGS_ARE_EQUAL(expected_frame_count, ring_buf.count);
    TEST_ASSERT_LONGS_ARE_EQUAL(expected_frames_dequeued, frames_dequeued);
    TEST_ASSERT_PTRS_ARE_EQUAL(expected_set_ptr, ring_buf.set_ptr);
    TEST_ASSERT_PTRS_ARE_EQUAL(expected_get_ptr, ring_buf.get_ptr);
}

void verify_dequeuing_partially_reports_not_empty(uint32_t frame_offset)
{
    const uint32_t init_buf_full_state = 1;
    const uint32_t init_buf_empty_state = 0;
    const uint32_t init_buf_count = appconfAUDIO_PIPELINE_FRAME_ADVANCE * appconfAUDIO_PIPELINE_BUFFER_NUM_PACKETS;
    uint32_t max_dequeue_packets = appconfAUDIO_PIPELINE_BUFFER_NUM_PACKETS - 1;
    //uint32_t starting_frame_value = 1;
    uint8_t expected_full_state = 0;
    uint8_t expected_empty_state = 0;
    uint32_t expected_frame_count = appconfAUDIO_PIPELINE_FRAME_ADVANCE;
    uint32_t expected_frames_dequeued = max_dequeue_packets * appconfAUDIO_PIPELINE_FRAME_ADVANCE;
    char *expected_set_ptr = (char *)(sample_buf + frame_offset);
    char *expected_get_ptr = (char *)(sample_buf + (
        (frame_offset + max_dequeue_packets * appconfAUDIO_PIPELINE_FRAME_ADVANCE) %
        (appconfAUDIO_PIPELINE_FRAME_ADVANCE * appconfAUDIO_PIPELINE_BUFFER_NUM_PACKETS)));
    set_buffer_state(expected_set_ptr,
                     (char *)(sample_buf + frame_offset),
                     init_buf_count,
                     init_buf_full_state,
                     init_buf_empty_state);

    //memset(frames, 0xFF, sizeof(frames));
    //fill_frames((int32_t *)frames, frames_to_enqueue, starting_frame_value);

    TEST_CASE_PRINTF("Verify dequeuing %ld of the %d enqueued packets at frame offset %ld.",
        max_dequeue_packets,
        appconfAUDIO_PIPELINE_BUFFER_NUM_PACKETS,
        frame_offset);
    //low_power_audio_buffer_enqueue((int32_t *)frames, frames_to_enqueue);
    uint32_t frames_dequeued = low_power_audio_buffer_dequeue(max_dequeue_packets);

    TEST_ASSERT_INTS_ARE_EQUAL(expected_full_state, ring_buf.full);
    TEST_ASSERT_INTS_ARE_EQUAL(expected_empty_state, ring_buf.empty);
    TEST_ASSERT_LONGS_ARE_EQUAL(expected_frame_count, ring_buf.count);
    TEST_ASSERT_LONGS_ARE_EQUAL(expected_frames_dequeued, frames_dequeued);
    TEST_ASSERT_PTRS_ARE_EQUAL(expected_set_ptr, ring_buf.set_ptr);
    TEST_ASSERT_PTRS_ARE_EQUAL(expected_get_ptr, ring_buf.get_ptr);
}

void verify_dequeuing_all_packets_reports_empty(uint32_t frame_offset)
{
    const uint32_t init_buf_full_state = 1;
    const uint32_t init_buf_empty_state = 0;
    const uint32_t init_buf_count = appconfAUDIO_PIPELINE_FRAME_ADVANCE * appconfAUDIO_PIPELINE_BUFFER_NUM_PACKETS;
    uint32_t max_dequeue_packets = appconfAUDIO_PIPELINE_BUFFER_NUM_PACKETS;
    //uint32_t starting_frame_value = 1;
    uint8_t expected_full_state = 0;
    uint8_t expected_empty_state = 1;
    uint32_t expected_frame_count = 0;
    uint32_t expected_frames_dequeued = max_dequeue_packets * appconfAUDIO_PIPELINE_FRAME_ADVANCE;
    char *expected_set_ptr = (char *)(sample_buf + frame_offset);
    char *expected_get_ptr = (char *)(sample_buf + (
        (frame_offset + max_dequeue_packets * appconfAUDIO_PIPELINE_FRAME_ADVANCE) %
        (appconfAUDIO_PIPELINE_FRAME_ADVANCE * appconfAUDIO_PIPELINE_BUFFER_NUM_PACKETS)));
    set_buffer_state(expected_set_ptr,
                     (char *)(sample_buf + frame_offset),
                     init_buf_count,
                     init_buf_full_state,
                     init_buf_empty_state);
    init_buffer();

    //memset(frames, 0xFF, sizeof(frames));
    //fill_frames((int32_t *)frames, frames_to_enqueue, starting_frame_value);

    TEST_CASE_PRINTF("Verify dequeuing all %ld enqueued packets at frame offset %ld.",
        max_dequeue_packets,
        frame_offset);
    //low_power_audio_buffer_enqueue((int32_t *)frames, frames_to_enqueue);
    uint32_t frames_dequeued = low_power_audio_buffer_dequeue(max_dequeue_packets);

    TEST_ASSERT_INTS_ARE_EQUAL(expected_full_state, ring_buf.full);
    TEST_ASSERT_INTS_ARE_EQUAL(expected_empty_state, ring_buf.empty);
    TEST_ASSERT_LONGS_ARE_EQUAL(expected_frame_count, ring_buf.count);
    TEST_ASSERT_LONGS_ARE_EQUAL(expected_frames_dequeued, frames_dequeued);
    TEST_ASSERT_PTRS_ARE_EQUAL(expected_set_ptr, ring_buf.set_ptr);
    TEST_ASSERT_PTRS_ARE_EQUAL(expected_get_ptr, ring_buf.get_ptr);
}

int main(void)
{
    TEST_PRINTF("CONFIGURATION:\n");
    TEST_PRINTF("- (Enqueue) Frames: %d\n", appconfAUDIO_PIPELINE_FRAME_ADVANCE);
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
     * Enqueuing at least one frame results in the buffer no longer being empty.
     * The buffer remains not full when enqueuing frames less than the buffer's capacity.
     */
    verify_enqueuing_frames_less_than_buffer_capacity_remains_not_full(1);
    verify_enqueuing_frames_less_than_buffer_capacity_remains_not_full(appconfAUDIO_PIPELINE_FRAME_ADVANCE);
    verify_enqueuing_frames_less_than_buffer_capacity_remains_not_full(appconfAUDIO_PIPELINE_FRAME_ADVANCE * appconfAUDIO_PIPELINE_BUFFER_NUM_PACKETS - 1);

    /*
     * Enqueuing frames to match the capacity of the buffer should result in the
     * buffer asserting the full flag; this behavior should be not be impacted
     * by the initial offset where enqueuing begins.
     */
    verify_enqueueing_frames_equal_to_buffer_capacity_reports_full(0);
    verify_enqueueing_frames_equal_to_buffer_capacity_reports_full(1);
    verify_enqueueing_frames_equal_to_buffer_capacity_reports_full(appconfAUDIO_PIPELINE_FRAME_ADVANCE);
    verify_enqueueing_frames_equal_to_buffer_capacity_reports_full(appconfAUDIO_PIPELINE_FRAME_ADVANCE * appconfAUDIO_PIPELINE_BUFFER_NUM_PACKETS - 2);
    verify_enqueueing_frames_equal_to_buffer_capacity_reports_full(appconfAUDIO_PIPELINE_FRAME_ADVANCE * appconfAUDIO_PIPELINE_BUFFER_NUM_PACKETS - 1);

    /*
     * If the buffer is empty, no frame data should be released/reported by
     * low_power_audio_buffer_dequeue().
     */
    verify_dequeuing_empty_buffer_does_not_output_frames();

    /*
     * Data cannot be dequeued until at least appconfAUDIO_PIPELINE_FRAME_ADVANCE
     * frames are enqueued.
     */
    verify_dequeuing_non_full_packet_is_not_possible(1);
    verify_dequeuing_non_full_packet_is_not_possible(appconfAUDIO_PIPELINE_FRAME_ADVANCE - 1);

    verify_dequeuing_partially_reports_not_empty(0);
    verify_dequeuing_partially_reports_not_empty(appconfAUDIO_PIPELINE_FRAME_ADVANCE);
    verify_dequeuing_partially_reports_not_empty(appconfAUDIO_PIPELINE_FRAME_ADVANCE * (appconfAUDIO_PIPELINE_BUFFER_NUM_PACKETS - 2)); //TODO: consider changing arg name to starting_packet_index
    verify_dequeuing_partially_reports_not_empty(appconfAUDIO_PIPELINE_FRAME_ADVANCE * (appconfAUDIO_PIPELINE_BUFFER_NUM_PACKETS - 1));

    verify_dequeuing_all_packets_reports_empty(0);
    verify_dequeuing_all_packets_reports_empty(appconfAUDIO_PIPELINE_FRAME_ADVANCE);
    verify_dequeuing_all_packets_reports_empty(appconfAUDIO_PIPELINE_FRAME_ADVANCE * (appconfAUDIO_PIPELINE_BUFFER_NUM_PACKETS - 2)); //TODO: consider changing arg name to starting_packet_index
    verify_dequeuing_all_packets_reports_empty(appconfAUDIO_PIPELINE_FRAME_ADVANCE * (appconfAUDIO_PIPELINE_BUFFER_NUM_PACKETS - 1));

    // TODO:
    //TEST_PRINTF("* Verify the buffer overruns.\n");

    if (error_count == 0) {
        TEST_PRINTF("TEST: PASS\n");
    }

    return 0;
}
