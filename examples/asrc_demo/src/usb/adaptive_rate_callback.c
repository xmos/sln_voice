// Copyright 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <stdint.h>
#include <stdbool.h>

#include "rate_server.h"

#include "xmath/xmath.h"
#define TOTAL_TAIL_SECONDS 16
#define STORED_PER_SECOND 4

#include "tusb_config.h"
#include "app_conf.h"
#include "adaptive_rate_callback.h"

#define TOTAL_STORED                    (TOTAL_TAIL_SECONDS * STORED_PER_SECOND)
#define REF_CLOCK_TICKS_PER_SECOND      XS1_TIMER_HZ
#define REF_CLOCK_TICKS_PER_STORED_AVG  (REF_CLOCK_TICKS_PER_SECOND / STORED_PER_SECOND)


bool first_time[2] = {true, true};
volatile static bool data_seen[2] = {false, false};

void reset_state(uint32_t direction)
{
    first_time[direction] = true;
}


usb_rate_calc_info_t determine_USB_audio_rate(uint32_t timestamp,
                                    uint32_t data_length,
                                    uint32_t direction
#ifdef DEBUG_ADAPTIVE
                                                ,
                                    uint32_t * debug
#endif
)
{
    static uint32_t data_lengths[2][TOTAL_STORED];
    static uint32_t time_buckets[2][TOTAL_STORED];
    static uint32_t current_data_bucket_size[2];
    static uint32_t first_timestamp[2];
    static bool buckets_full[2];
    static uint32_t times_overflowed[2];

    data_length = data_length / (CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_RX * CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX); // Number of samples per channels per transaction


    if (data_seen[direction] == false)
    {
        data_seen[direction] = true;
    }

    if (first_time[direction])
    {
        first_time[direction] = false;
        first_timestamp[direction] = timestamp;

        // Because we use "first_time" to also reset the rate determinator,
        // reset all the static variables to default.
        current_data_bucket_size[direction] = 0;
        times_overflowed[direction] = 0;
        buckets_full[direction] = false;

        for (int i = 0; i < TOTAL_STORED; i++)
        {
            data_lengths[direction][i] = 0;
            time_buckets[direction][i] = 0;
        }

        return (usb_rate_calc_info_t){appconfUSB_AUDIO_SAMPLE_RATE, REF_CLOCK_TICKS_PER_SECOND};
    }

    current_data_bucket_size[direction] += data_length;



    // total_timespan is always correct regardless of whether the reference clock has overflowed.
    // The point at which it becomes incorrect is the point at which it would overflow - the
    // point at which timestamp == first_timestamp again. This will be at 42.95 seconds of operation.
    // If current_data_bucket_size overflows we have bigger issues, so this case is not guarded.

    uint32_t timespan = timestamp - first_timestamp[direction];


    usb_rate_calc_info_t result = {0, 0};
    uint32_t total_data_intermed = current_data_bucket_size[direction] + sum_array(data_lengths[direction], TOTAL_STORED);
    uint32_t total_timespan = timespan + sum_array(time_buckets[direction], TOTAL_STORED);
    result.total_data_samples = total_data_intermed;
    result.total_ticks = total_timespan;


    if (timespan >= REF_CLOCK_TICKS_PER_STORED_AVG)
    {
        if (buckets_full[direction])
        {
            // We've got enough data for a new bucket - replace the oldest bucket data with this new data
            uint32_t oldest_bucket = times_overflowed[direction] % TOTAL_STORED;

            time_buckets[direction][oldest_bucket] = timespan;
            data_lengths[direction][oldest_bucket] = current_data_bucket_size[direction];

            current_data_bucket_size[direction] = 0;
            first_timestamp[direction] = timestamp;

            times_overflowed[direction]++;
        }
        else
        {
            // We've got enough data for this bucket - save this one and start the next one
            time_buckets[direction][times_overflowed[direction]] = timespan;
            data_lengths[direction][times_overflowed[direction]] = current_data_bucket_size[direction];

            current_data_bucket_size[direction] = 0;
            first_timestamp[direction] = timestamp;

            times_overflowed[direction]++;
            if (times_overflowed[direction] == TOTAL_STORED)
            {
                buckets_full[direction] = true;
            }
        }
    }

#ifdef DEBUG_ADAPTIVE
    #define DEBUG_QUANT 4

    uint32_t debug_out[DEBUG_QUANT] = {result, data_per_sample, total_data_intermed, total_timespan};
    for (int i = 0; i < DEBUG_QUANT; i++)
    {
        debug[i] = debug_out[i];
    }
#endif
    return result;
}

void sof_toggle()
{
    static uint32_t sof_count[2];

    for(uint32_t dir=0; dir<2; dir++)
    {
        if (data_seen[dir])
        {
            sof_count[dir] = 0;
            data_seen[dir] = false;
        }
        else
        {
            sof_count[dir]++;
            if (sof_count[dir] > 8)
            {
                reset_state(dir);
            }
        }
    }
}
