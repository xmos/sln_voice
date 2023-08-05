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
#include "rate_server.h"

#define TOTAL_TAIL_SECONDS (16)
#define STORED_PER_SECOND (4)

#define TOTAL_STORED (TOTAL_TAIL_SECONDS * STORED_PER_SECOND)
#define REF_CLOCK_TICKS_PER_SECOND 100000000
#define REF_CLOCK_TICKS_PER_STORED_AVG (REF_CLOCK_TICKS_PER_SECOND / STORED_PER_SECOND)

volatile static bool data_seen = false;
volatile static bool hold_average = false;


extern uint32_t dsp_math_divide_unsigned_64(uint64_t dividend, uint32_t divisor, uint32_t q_format );
extern uint32_t sum_array(uint32_t * array_to_sum, uint32_t array_length);
extern uint32_t dsp_math_divide_unsigned(uint32_t dividend, uint32_t divisor, uint32_t q_format );

// Global variables shared with i2s_audio.c
uint32_t g_i2s_to_usb_rate_ratio = 0;
uint32_t g_i2s_nominal_sampling_rate = 0;

static inline bool in_range(uint32_t ticks, uint32_t ref)
{
    if((ticks >= (ref-5)) && (ticks <= (ref+5)))
    {
        return true;
    }
    else
    {
        return false;
    }
}

static inline uint32_t detect_i2s_sampling_rate(uint32_t average_callback_ticks)
{
    if(in_range(average_callback_ticks, 2267))
    {
        return 44100;
    }
    else if(in_range(average_callback_ticks, 2083))
    {
        return 48000;
    }
    else if(in_range(average_callback_ticks, 1133))
    {
        return 88200;
    }
    else if(in_range(average_callback_ticks, 1041))
    {
        return 96000;
    }
    else if(in_range(average_callback_ticks, 566))
    {
        return 176400;
    }
    else if(in_range(average_callback_ticks, 520))
    {
        return 192000;
    }
    else if(average_callback_ticks == 0)
    {
        return 0;
    }
    printf("ERROR: avg_callback_ticks %lu do not match any sampling rate!!\n", average_callback_ticks);
    xassert(0);
    return 0xffffffff;
}

#define OLD_VAL_WEIGHTING (5)

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
    static uint32_t previous_result = 0;
    static uint32_t prev_nominal_sampling_rate = 0;
    static uint32_t expected_nominal_samples_per_bucket = 0;

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

    g_i2s_nominal_sampling_rate = detect_i2s_sampling_rate(i2s_ctx->average_callback_time);
    if(g_i2s_nominal_sampling_rate == 0)
    {
        return 0; // i2s_audio thread ensures we don't do asrc when g_i2s_nominal_sampling_rate is 0
    }
    else if(g_i2s_nominal_sampling_rate != prev_nominal_sampling_rate)
    {
        expected_nominal_samples_per_bucket = g_i2s_nominal_sampling_rate / STORED_PER_SECOND; // samples_per_second / number_of_buckets_per_second

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
            data_lengths[i] = expected_nominal_samples_per_bucket;
            time_buckets[i] = REF_CLOCK_TICKS_PER_STORED_AVG;
        }
        prev_nominal_sampling_rate = g_i2s_nominal_sampling_rate;
        previous_result = dsp_math_divide_unsigned_64(g_i2s_nominal_sampling_rate, 1000, SAMPLING_RATE_Q_FORMAT); // Samples per ms in SAMPLING_RATE_Q_FORMAT format

        return previous_result;
    }

    if (update)
    {
        current_data_bucket_size += data_length;
    }

    uint32_t timespan = timestamp - first_timestamp;

    uint32_t total_data_intermed = current_data_bucket_size + sum_array(data_lengths, TOTAL_STORED);
    uint64_t total_data = (uint64_t)(total_data_intermed) * 12500;
    uint32_t total_timespan = timespan + sum_array(time_buckets, TOTAL_STORED);

    uint32_t data_per_sample = dsp_math_divide_unsigned_64(total_data, (total_timespan / 8), SAMPLING_RATE_Q_FORMAT); // This is how much I'm getting from I2S every millisecond


    uint32_t result = data_per_sample;

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

#define BUFFER_LEVEL_TERM   200000   //How much to apply the buffer level feedback term (effectively 1/I term)
void rate_server(void *args)
{
    unsigned fs_ratio_i2s_to_usb_old = 0;
    unsigned fs_ratio_usb_to_i2s_old = 0;
    uint32_t prev_ts = get_reference_time();
    uint32_t prev_num_i2s_samples_recvd = i2s_ctx->recv_buffer.total_written;
    uint32_t usb_to_i2s_rate_ratio = 0;
    int32_t usb_buffer_fill_level_from_half;
    usb_to_i2s_rate_info_t usb_rate_info;
    for(;;)
    {
        vTaskDelay(pdMS_TO_TICKS(20));
        uint32_t current_ts = get_reference_time();

        size_t i2s_send_buffer_unread = i2s_ctx->send_buffer.total_written - i2s_ctx->send_buffer.total_read;
        int i2s_buffer_level_from_half = (signed)i2s_send_buffer_unread - (i2s_ctx->send_buffer.buf_size / 2);    //Level w.r.t. half full

        uint32_t current_num_i2s_samples = i2s_ctx->recv_buffer.total_written;

        uint32_t samples = (current_num_i2s_samples - prev_num_i2s_samples_recvd) >> 1; // 2 channels per sample
        uint32_t i2s_rate = determine_I2S_rate(current_ts, samples, true);


        prev_num_i2s_samples_recvd = current_num_i2s_samples;
        prev_ts = current_ts;

        // Get USB rate and buffer information from the other tile
        rtos_intertile_tx(
            intertile_ctx,
            appconfUSB_RATE_NOTIFY_PORT,
            &g_i2s_nominal_sampling_rate,
            sizeof(g_i2s_nominal_sampling_rate));

        size_t bytes_received;
        uint32_t usb_rate;
        bytes_received = rtos_intertile_rx_len(
                    intertile_ctx,
                    appconfUSB_RATE_NOTIFY_PORT,
                    portMAX_DELAY);
        xassert(bytes_received == sizeof(usb_rate_info));

        rtos_intertile_rx_data(
                        intertile_ctx,
                        &usb_rate_info,
                        bytes_received);

        usb_rate = (uint32_t)usb_rate_info.usb_data_rate;
        usb_buffer_fill_level_from_half = usb_rate_info.samples_to_host_buf_fill_level;

        // Calculate g_i2s_to_usb_rate_ratio only when we're streaming out
        if((i2s_rate != 0) && (usb_rate_info.spkr_itf_open))
        {
            printintln(usb_buffer_fill_level_from_half);
            int32_t fs_ratio;
            fs_ratio_i2s_to_usb_old = g_i2s_to_usb_rate_ratio;
            fs_ratio = dsp_math_divide_unsigned_64(i2s_rate, usb_rate, 28); // Samples per millisecond
            //printf("i2s_to_usb_ratio = %lu\n", g_i2s_to_usb_rate_ratio);

            fs_ratio = (unsigned) (((BUFFER_LEVEL_TERM + usb_buffer_fill_level_from_half) * (unsigned long long)fs_ratio) / BUFFER_LEVEL_TERM);

            /*fs_ratio = (unsigned) (((unsigned long long)(fs_ratio_i2s_to_usb_old) * OLD_VAL_WEIGHTING + (unsigned long long)(fs_ratio) ) /
                            (1 + OLD_VAL_WEIGHTING));*/

            g_i2s_to_usb_rate_ratio = fs_ratio;
        }
        else
        {
            g_i2s_to_usb_rate_ratio = 0;
        }

        // Calculate usb_to_i2s_rate_ratio only when we're recording
        if((i2s_rate != 0) && (usb_rate_info.mic_itf_open))
        {
            //int32_t nom_rate = dsp_math_divide_unsigned_64(g_i2s_nominal_sampling_rate, 1000, SAMPLING_RATE_Q_FORMAT); // Samples per ms in SAMPLING_RATE_Q_FORMAT format
            int32_t fs_ratio;
            fs_ratio_usb_to_i2s_old = usb_to_i2s_rate_ratio;
            fs_ratio = dsp_math_divide_unsigned_64(usb_rate, i2s_rate, 28); // Samples per millisecond
            //fs_ratio = 0x40000000;
            //printchar(',');
            //printhex(fs_ratio);
            fs_ratio = (unsigned) (((BUFFER_LEVEL_TERM + i2s_buffer_level_from_half) * (unsigned long long)fs_ratio) / BUFFER_LEVEL_TERM);
            /*fs_ratio = (unsigned) (((unsigned long long)(fs_ratio_usb_to_i2s_old) * OLD_VAL_WEIGHTING + (unsigned long long)(fs_ratio) ) /
                            (1 + OLD_VAL_WEIGHTING));*/
            usb_to_i2s_rate_ratio = fs_ratio;
            //printchar(',');
            //printhexln(usb_to_i2s_rate_ratio);
            //printf("usb_to_i2s_rate_ratio = %f\n", (float)usb_to_i2s_rate_ratio/(1<<28));

        }
        else
        {
            usb_to_i2s_rate_ratio = 0;
        }

        rtos_intertile_tx(
            intertile_ctx,
            appconfUSB_RATE_NOTIFY_PORT,
            &usb_to_i2s_rate_ratio,
            sizeof(usb_to_i2s_rate_ratio));


        // Get the I2S rate

    }
}
