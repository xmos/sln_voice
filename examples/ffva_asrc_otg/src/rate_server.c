#define DEBUG_UNIT RATE_SERVER
#define DEBUG_PRINT_ENABLE_RATE_SERVER 1

/* STD headers */
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "rtos_printf.h"
#include <xcore/hwtimer.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "stream_buffer.h"

/* App headers */
#include "app_conf.h"
#include "platform/driver_instances.h"
#include "src.h"
#include "asrc_utils.h"
#include "i2s_audio.h"

#define TOTAL_TAIL_SECONDS (16)
#define STORED_PER_SECOND (4)
#define RATE_SERVER_TIME_PERIOD_MS (20)
#define NUM_RATE_SERVER_PERIODS_PER_SECOND (1000/RATE_SERVER_TIME_PERIOD_MS)

#define EXPECTED_SAMPLES_PER_TRANSACTION (192 * RATE_SERVER_TIME_PERIOD_MS) // TODO. Detect dynamically
#define TOTAL_STORED (TOTAL_TAIL_SECONDS * STORED_PER_SECOND)
#define REF_CLOCK_TICKS_PER_SECOND 100000000
#define REF_CLOCK_TICKS_PER_STORED_AVG (REF_CLOCK_TICKS_PER_SECOND / STORED_PER_SECOND)
#define NOMINAL_RATE (1 << 31)

#define EXPECTED_SAMPLES_PER_BUCKET ((EXPECTED_SAMPLES_PER_TRANSACTION * NUM_RATE_SERVER_PERIODS_PER_SECOND) / STORED_PER_SECOND)

static bool first_time = true;
volatile static bool data_seen = false;
volatile static bool hold_average = false;
static uint32_t expected = EXPECTED_SAMPLES_PER_TRANSACTION;
static uint32_t bucket_expected = EXPECTED_SAMPLES_PER_BUCKET;

extern uint32_t dsp_math_divide_unsigned_64(uint64_t dividend, uint32_t divisor, uint32_t q_format );
extern uint32_t sum_array(uint32_t * array_to_sum, uint32_t array_length);
extern uint32_t dsp_math_divide_unsigned(uint32_t dividend, uint32_t divisor, uint32_t q_format );

static uint32_t determine_I2S_rate(
    uint32_t timestamp,
    uint32_t data_length,
    bool update
    )
{
    static uint32_t data_lengths[TOTAL_STORED];
    static uint32_t time_buckets[TOTAL_STORED];
    static uint32_t current_data_bucket_size;
    static uint32_t first_timestamp;
    static bool buckets_full;
    static uint32_t times_overflowed;
    static uint32_t previous_result = NOMINAL_RATE;

    if (data_seen == false)
    {
        data_seen = true;
    }
    if (hold_average)
    {
        hold_average = false;
        first_timestamp = timestamp;
        current_data_bucket_size = 0;
        return previous_result;
    }
    if (first_time)
    {
        first_time = false;
        first_timestamp = timestamp;

        // Because we use "first_time" to also reset the rate determinator,
        // reset all the static variables to default.
        current_data_bucket_size = 0;
        times_overflowed = 0;
        buckets_full = false;

        for (int i = 0; i < TOTAL_STORED - STORED_PER_SECOND; i++)
        {
            data_lengths[i] = 0;
            time_buckets[i] = 0;
        }
        // Seed the final second of initialised data with a "perfect" second - should make the start a bit more stable
        for (int i = TOTAL_STORED - STORED_PER_SECOND; i < TOTAL_STORED; i++)
        {
            data_lengths[i] = bucket_expected;
            time_buckets[i] = REF_CLOCK_TICKS_PER_STORED_AVG;
        }

        return NOMINAL_RATE;
    }

    if (update)
    {
        current_data_bucket_size += data_length;
    }

    uint32_t timespan = timestamp - first_timestamp;

    uint32_t total_data_intermed = current_data_bucket_size + sum_array(data_lengths, TOTAL_STORED);
    uint64_t total_data = (uint64_t)(total_data_intermed) * 12500;
    uint32_t total_timespan = timespan + sum_array(time_buckets, TOTAL_STORED);

    uint32_t data_per_sample = dsp_math_divide_unsigned_64(total_data, (total_timespan / 8), 19);
    printf("data_per_sample = %f\n", (float)data_per_sample/(1<<19));
    uint32_t result = dsp_math_divide_unsigned(data_per_sample, expected, 12);

    printuintln(result);

    if (update && (timespan >= REF_CLOCK_TICKS_PER_STORED_AVG))
    {
        if (buckets_full)
        {
            // We've got enough data for a new bucket - replace the oldest bucket data with this new data
            uint32_t oldest_bucket = times_overflowed % TOTAL_STORED;

            time_buckets[oldest_bucket] = timespan;
            data_lengths[oldest_bucket] = current_data_bucket_size;

            current_data_bucket_size = 0;
            first_timestamp = timestamp;

            times_overflowed++;
        }
        else
        {
            // We've got enough data for this bucket - save this one and start the next one
            time_buckets[times_overflowed] = timespan;
            data_lengths[times_overflowed] = current_data_bucket_size;

            current_data_bucket_size = 0;
            first_timestamp = timestamp;

            times_overflowed++;
            if (times_overflowed == TOTAL_STORED)
            {
                buckets_full = true;
            }
        }
    }
    previous_result = result;
    return result;
}

void rate_server(void *args)
{
    uint32_t prev_ts = get_reference_time();
    uint32_t prev_num_i2s_samples_recvd = i2s_ctx->recv_buffer.total_written;
    for(;;)
    {
        vTaskDelay(pdMS_TO_TICKS(20));
        uint32_t current_ts = get_reference_time();
        uint32_t current_num_i2s_samples = i2s_ctx->recv_buffer.total_written;

        uint32_t samples = (current_num_i2s_samples - prev_num_i2s_samples_recvd) >> 1; // 2 channels per sample, 4 bytes per channel
        uint32_t rate_ratio = determine_I2S_rate(current_ts, samples, true);

        //printuintln(rate_ratio);

        //uint32_t timespan = current_ts - prev_ts;
        //uint64_t total_data = (uint64_t)(samples) * 12500;
        //uint32_t data_per_sample = dsp_math_divide_unsigned_64(total_data, (timespan / 8), 19);

        //printuintln(data_per_sample);
        //printuintln(samples);
        //printuintln(current_ts - prev_ts);

        prev_num_i2s_samples_recvd = current_num_i2s_samples;
        prev_ts = current_ts;

        uint8_t tmp = 0;
        rtos_intertile_tx(
            intertile_ctx,
            appconfUSB_RATE_NOTIFY_PORT,
            &tmp,
            sizeof(tmp));

        size_t bytes_received;
        uint32_t usb_rate_ratio;
        bytes_received = rtos_intertile_rx_len(
                    intertile_ctx,
                    appconfUSB_RATE_NOTIFY_PORT,
                    portMAX_DELAY);
        xassert(bytes_received == sizeof(usb_rate_ratio));

        rtos_intertile_rx_data(
                        intertile_ctx,
                        &usb_rate_ratio,
                        bytes_received);

        // Get the I2S rate

    }
}
