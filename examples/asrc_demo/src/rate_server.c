// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#define DEBUG_UNIT RATE_SERVER
#define DEBUG_PRINT_ENABLE_RATE_SERVER 0

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
#include "avg_buffer_level.h"
#include "tusb.h"
#include "div.h"

#define LOG_I2S_TO_USB_SIDE (0)
#define LOG_USB_TO_I2S_SIDE (0)

#define REF_CLOCK_TICKS_PER_SECOND 100000000

static uint64_t g_i2s_to_usb_rate_ratio = 0; // i2s_to_usb_rate_ratio. Updated in rate monitor and used in i2s_audio_recv_task
static bool g_spkr_itf_close_to_open = false; // Flag tracking if a USB spkr interface close->open event occured. Set in the rate monitor when it receives the spkr_interface info from
                                       // USB. Cleared in usb_to_i2s_intertile, after it resets the i2s send buffer

static buffer_calc_state_t g_i2s_send_buf_state;

bool get_spkr_itf_close_open_event()
{
    return g_spkr_itf_close_to_open;
}

void set_spkr_itf_close_open_event(bool event)
{
    g_spkr_itf_close_to_open = event;
}


uint64_t get_i2s_to_usb_rate_ratio()
{
    return g_i2s_to_usb_rate_ratio;
}

void set_i2s_to_usb_rate_ratio(uint64_t ratio)
{
    g_i2s_to_usb_rate_ratio = ratio;

}

// Wrapper functions to avoid having g_i2s_send_buf_state visible in i2s_audio.c
void init_calc_i2s_buffer_level_state(void)
{
   // The window size and buffer_level_stable_threahold are calculated using the simulation
   // framework to ensure that they are large enough that we get stable windowed averages
    int32_t window_size_log2 = 10;
    init_calc_buffer_level_state(&g_i2s_send_buf_state, window_size_log2, 8);
}

void calc_avg_i2s_send_buffer_level(int32_t current_buffer_level, bool reset)
{
    calc_avg_buffer_level(&g_i2s_send_buf_state, current_buffer_level, reset);
}

static float_s32_t determine_avg_I2S_rate_from_driver()
{
    #define TOTAL_STORED_AVG_I2S_RATE (16)
    static uint32_t data_lengths[TOTAL_STORED_AVG_I2S_RATE];
    static uint32_t time_buckets[TOTAL_STORED_AVG_I2S_RATE];
    static uint32_t current_data_bucket_size;
    static bool buckets_full;
    static uint32_t times_overflowed;
    static float_s32_t previous_result = {.mant = 0, .exp = 0};
    static uint32_t prev_nominal_sampling_rate = 0;
    static uint32_t counter = 0;
    static uint32_t timespan_current_bucket = 0;

    uint32_t timespan;
    uint32_t num_samples;
    rtos_i2s_get_current_rate_info(i2s_ctx, &timespan, &num_samples);

    uint32_t i2s_nominal_sampling_rate = rtos_i2s_get_nominal_sampling_rate(i2s_ctx);
    if(i2s_nominal_sampling_rate == 0)
    {
        float_s32_t t = {.mant=0, .exp=0};
        return t;
    }
    else if(i2s_nominal_sampling_rate != prev_nominal_sampling_rate)
    {
        rtos_printf("determine_avg_I2S_rate_from_driver() I2S SR change detected, new_sr = %lu, prev_sr = %lu\n", i2s_nominal_sampling_rate, prev_nominal_sampling_rate);
        counter = 0;
        timespan_current_bucket = 0;

        // Because we use "first_time" to also reset the rate determinator,
        // reset all the static variables to default.
        current_data_bucket_size = 0;
        times_overflowed = 0;
        buckets_full = false;

        for (int i = 0; i < TOTAL_STORED_AVG_I2S_RATE; i++)
        {
            data_lengths[i] = 0;
            time_buckets[i] = 0;
        }
        prev_nominal_sampling_rate = i2s_nominal_sampling_rate;
        float_s32_t a = {.mant=i2s_nominal_sampling_rate, .exp=0};
        float_s32_t b = {.mant=REF_CLOCK_TICKS_PER_SECOND, .exp=0};

        float_s32_t rate = float_div(a, b);
        previous_result = rate;

        return previous_result;
    }
    else if(timespan == 0)
    {
        float_s32_t a = {.mant=prev_nominal_sampling_rate, .exp=0};
        float_s32_t b = {.mant=REF_CLOCK_TICKS_PER_SECOND, .exp=0};

        float_s32_t rate = float_div(a, b);
        previous_result = rate;

        return previous_result;

    }

    counter += 1;

    current_data_bucket_size += num_samples;
    timespan_current_bucket += timespan;

    uint32_t total_data_intermed = current_data_bucket_size + sum_array(data_lengths, TOTAL_STORED_AVG_I2S_RATE);
    uint32_t total_timespan = timespan_current_bucket + sum_array(time_buckets, TOTAL_STORED_AVG_I2S_RATE);

    float_s32_t data_per_sample = float_div((float_s32_t){total_data_intermed, 0}, (float_s32_t){total_timespan, 0});

    //float_s32_t data_per_sample1 = float_div((float_s32_t){num_samples, 0}, (float_s32_t){timespan, 0});

    float_s32_t result = data_per_sample;

    if (counter >= 16)
    {
        if (buckets_full)
        {
            // We've got enough data for a new bucket - replace the oldest bucket data with this new data
            uint32_t oldest_bucket = times_overflowed % TOTAL_STORED_AVG_I2S_RATE;

            time_buckets[oldest_bucket] = timespan_current_bucket;
            data_lengths[oldest_bucket] = current_data_bucket_size;

            current_data_bucket_size = 0;
            counter = 0;
            timespan_current_bucket = 0;

            times_overflowed++;
        }
        else
        {
            // We've got enough data for this bucket - save this one and start the next one
            time_buckets[times_overflowed] = timespan_current_bucket;
            data_lengths[times_overflowed] = current_data_bucket_size;

            current_data_bucket_size = 0;
            counter = 0;
            timespan_current_bucket = 0;

            times_overflowed++;
            if (times_overflowed == TOTAL_STORED_AVG_I2S_RATE)
            {
                buckets_full = true;
            }
        }
    }
    previous_result = result;
    return result;
}

static inline sw_pll_q24_t get_Kp_for_i2s_buffer_control(int32_t nominal_i2s_rate)
{
    // The Kp constants are generated using the simulation framework empirically, to get values using which
    // the calculated correction factor stablises the buffer level.
    sw_pll_q24_t Kp = 0;
    if(((int)nominal_i2s_rate == (int)44100) || ((int)nominal_i2s_rate == (int)48000))
    {
        Kp = KP_I2S_BUF_CONTROL_FS48;
    }
    else if(((int)nominal_i2s_rate == (int)88200) || ((int)nominal_i2s_rate == (int)96000))
    {
        Kp = KP_I2S_BUF_CONTROL_FS96;
    }
    else if(((int)nominal_i2s_rate == (int)176400) || ((int)nominal_i2s_rate == (int)192000))
    {
        Kp = KP_I2S_BUF_CONTROL_FS192;
    }
    return Kp;
}

void rate_server(void *args)
{
    static bool prev_spkr_itf_open = false;
    uint64_t usb_to_i2s_rate_ratio = 0;
    usb_rate_info_t usb_rate_info;
    i2s_to_usb_rate_info_t i2s_rate_info;

    for(;;)
    {
        // Get usb_rate_info from the other tile
        size_t bytes_received;
        float_s32_t usb_rate;
        bytes_received = rtos_intertile_rx_len(
                    intertile_ctx,
                    appconfUSB_RATE_NOTIFY_PORT,
                    portMAX_DELAY);
        xassert(bytes_received == sizeof(usb_rate_info));

        rtos_intertile_rx_data(
                        intertile_ctx,
                        &usb_rate_info,
                        bytes_received);

        usb_rate = usb_rate_info.usb_data_rate;

        if((prev_spkr_itf_open == false) && (usb_rate_info.spkr_itf_open == true))
        {
            set_spkr_itf_close_open_event(true);
        }
        prev_spkr_itf_open = usb_rate_info.spkr_itf_open;

        // Compute I2S rate
        float_s32_t i2s_rate = determine_avg_I2S_rate_from_driver();

        // Calculate g_i2s_to_usb_rate_ratio only when the host is recording data from the device
        if((i2s_rate.mant != 0) && (usb_rate.mant != 0) && (usb_rate_info.mic_itf_open))
        {
            uint64_t fs_ratio_u64 = float_div_u64_fixed_output_q_format(i2s_rate, usb_rate, 28+32);
            fs_ratio_u64 = fs_ratio_u64 + usb_rate_info.buffer_based_correction;

#if LOG_I2S_TO_USB_SIDE
            printint(usb_rate_info.samples_to_host_buf_fill_level);
            printchar(',');
            //printintln((uint32_t)(fs_ratio_u64 >> 32));
            printintln((int32_t)(usb_rate_info.buffer_based_correction >> 32));

#endif

            set_i2s_to_usb_rate_ratio(fs_ratio_u64);
        }
        else
        {
            set_i2s_to_usb_rate_ratio((uint64_t)0);
        }

        // Calculate usb_to_i2s_rate_ratio only when the host is playing data to the device
        if((i2s_rate.mant != 0) && (usb_rate.mant != 0) && (usb_rate_info.spkr_itf_open))
        {
            const sw_pll_q24_t Kp = get_Kp_for_i2s_buffer_control(rtos_i2s_get_nominal_sampling_rate(i2s_ctx));
            int64_t max_allowed_correction = (int64_t)1500 << 32;
            int64_t total_error = 0;

            uint64_t fs_ratio64 = float_div_u64_fixed_output_q_format(usb_rate, i2s_rate, 28+32);

            if(g_i2s_send_buf_state.flag_stable_avg)
            {
                int64_t error_p = ((int64_t)Kp * (int64_t)(g_i2s_send_buf_state.avg_buffer_level - g_i2s_send_buf_state.stable_avg_level));
                total_error = (int64_t)(error_p << 8);
                if(total_error > max_allowed_correction)
                {
                    total_error = max_allowed_correction;
                }
                else if(total_error < -(max_allowed_correction))
                {
                    total_error = -(max_allowed_correction);
                }
#if LOG_USB_TO_I2S_SIDE
            printint(g_i2s_send_buf_state.avg_buffer_level);
            printchar(',');
            printintln((int32_t)(total_error >> 32)); // Print the upper 32 bits of the correction
#endif
            }
            usb_to_i2s_rate_ratio = fs_ratio64 + total_error;

        }
        else
        {
            usb_to_i2s_rate_ratio = (uint64_t)0;
        }

        // Notify USB tile of the usb_to_i2s rate ratio
        i2s_rate_info.usb_to_i2s_rate_ratio = usb_to_i2s_rate_ratio;

        rtos_intertile_tx(
            intertile_ctx,
            appconfUSB_RATE_NOTIFY_PORT,
            &i2s_rate_info,
            sizeof(i2s_rate_info));
    }
}


uint32_t sum_array(uint32_t * array_to_sum, uint32_t array_length)
{
    uint32_t acc = 0;
    for (uint32_t i = 0; i < array_length; i++)
    {
        acc += array_to_sum[i];
    }
    return acc;
}
