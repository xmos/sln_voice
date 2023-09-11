// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
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
#include "tusb.h"

#define LOG_I2S_TO_USB_SIDE (0)
#define LOG_USB_TO_I2S_SIDE (1)

#define REF_CLOCK_TICKS_PER_SECOND 100000000

static uint64_t g_i2s_to_usb_rate_ratio = 0; // i2s_to_usb_rate_ratio. Updated in rate monitor and used in i2s_audio_recv_task
static int32_t g_avg_i2s_send_buffer_level = 0; // avg i2s send buffer level. Updated in usb_to_i2s_intertile(), used in rate monitor for buffer level based PI control
static int32_t g_prev_avg_i2s_send_buffer_level = 0; // Previous avg i2s send buffer level. Updated in usb_to_i2s_intertile(), used in rate monitor for buffer level based PI control
static bool g_spkr_itf_close_to_open = false; // Flag tracking if a USB spkr interface close->open event occured. Set in the rate monitor when it receives the spkr_interface info from
                                       // USB. Cleared in usb_to_i2s_intertile, after it resets the i2s send buffer

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

void calc_avg_i2s_send_buffer_level(int current_level, bool reset)
{
    static int64_t error_accum = 0;
    static int32_t count = 0;

    if(reset == true)
    {
        error_accum = 0;
        count = 0;
        g_avg_i2s_send_buffer_level = 0;
        g_prev_avg_i2s_send_buffer_level = 0;
        rtos_printf("Reset avg I2S send buffer level\n");
    }

    error_accum += current_level;
    count += 1;
    uint32_t avg_window_log2 = 10;
    if(count == (1 << avg_window_log2))
    {
        g_prev_avg_i2s_send_buffer_level = g_avg_i2s_send_buffer_level;
        g_avg_i2s_send_buffer_level = error_accum >> avg_window_log2;
        count = 0;
        error_accum = 0;
    }
}

typedef int32_t sw_pll_15q16_t; // Type for 15.16 signed fixed point
#define SW_PLL_NUM_FRAC_BITS 16
#define SW_PLL_15Q16(val) ((sw_pll_15q16_t)((float)val * (1 << SW_PLL_NUM_FRAC_BITS)))

#define BUFFER_LEVEL_TERM (400000)   //How much to apply the buffer level feedback term (effectively 1/I term)

void rate_server(void *args)
{
    uint64_t usb_to_i2s_rate_ratio = 0;
    int32_t usb_buffer_fill_level_from_half;
    static bool reset_buf_level = false;
    static bool prev_spkr_itf_open = false;
    usb_to_i2s_rate_info_t usb_rate_info;
    i2s_to_usb_rate_info_t i2s_rate_info;

    const sw_pll_15q16_t Ki = SW_PLL_15Q16(3);
    const sw_pll_15q16_t Kd = SW_PLL_15Q16(1);

    for(;;)
    {
        // Get usb_rate_info from the other tile
        size_t bytes_received;
        float_s32_t usb_rate[2];
        bytes_received = rtos_intertile_rx_len(
                    intertile_ctx,
                    appconfUSB_RATE_NOTIFY_PORT,
                    portMAX_DELAY);
        xassert(bytes_received == sizeof(usb_rate_info));

        rtos_intertile_rx_data(
                        intertile_ctx,
                        &usb_rate_info,
                        bytes_received);

        usb_rate[TUSB_DIR_OUT] = usb_rate_info.usb_data_rate[TUSB_DIR_OUT];
        usb_rate[TUSB_DIR_IN] = usb_rate_info.usb_data_rate[TUSB_DIR_IN];

        if((prev_spkr_itf_open == false) && (usb_rate_info.spkr_itf_open == true))
        {
            set_spkr_itf_close_open_event(true);
        }
        prev_spkr_itf_open = usb_rate_info.spkr_itf_open;

        // Compute I2S rate
        float_s32_t i2s_rate = determine_avg_I2S_rate_from_driver();

        usb_buffer_fill_level_from_half = usb_rate_info.samples_to_host_buf_fill_level / 8;

        // Calculate g_i2s_to_usb_rate_ratio only when the host is recording data from the device
        if((i2s_rate.mant != 0) && (usb_rate_info.mic_itf_open))
        {
            int32_t buffer_level_term = BUFFER_LEVEL_TERM;
            uint64_t fs_ratio_u64 = float_div_u64_fixed_output_q_format(i2s_rate, usb_rate[TUSB_DIR_IN], 28+32);

#if LOG_I2S_TO_USB_SIDE
            printint(usb_buffer_fill_level_from_half);
            printchar(',');
            printintln((uint32_t)(fs_ratio_u64 >> 32));
#endif
            // TODO fix guard band calculation
            /*int guard_level = 100;
            if(usb_buffer_fill_level_from_half > guard_level)
            {
                int error = usb_buffer_fill_level_from_half - guard_level;
                fs_ratio = (unsigned) (((buffer_level_term + error) * (unsigned long long)fs_ratio) / buffer_level_term);
            }
            else if(usb_buffer_fill_level_from_half < -guard_level)
            {
                int error = usb_buffer_fill_level_from_half - (-guard_level);
                fs_ratio = (unsigned) (((buffer_level_term + error) * (unsigned long long)fs_ratio) / buffer_level_term);
            }*/

            set_i2s_to_usb_rate_ratio(fs_ratio_u64);

            if(reset_buf_level)
            {
                reset_buf_level = false;
            }
        }
        else
        {
            set_i2s_to_usb_rate_ratio((uint64_t)0);
            reset_buf_level = true;
        }

        // Calculate usb_to_i2s_rate_ratio only when the host is playing data to the device
        if((i2s_rate.mant != 0) && (usb_rate_info.spkr_itf_open))
        {
            uint64_t fs_ratio64 = float_div_u64_fixed_output_q_format(usb_rate[TUSB_DIR_OUT], i2s_rate, 28+32);

            // TODO till figure out tuning
            /*int64_t error_d = ((int64_t)Kd * (int64_t)(g_avg_i2s_send_buffer_level - g_prev_avg_i2s_send_buffer_level));
            int64_t error_i = ((int64_t)Ki * (int64_t)g_avg_i2s_send_buffer_level);

            int32_t total_error = (int32_t)((error_d + error_i) >> SW_PLL_NUM_FRAC_BITS);
            if(total_error > 200)
            {
                total_error = 200;
            }
            else if(total_error < -200)
            {
                total_error = -200;
            }*/

            // This is still WIP so leaving this commented out code here
#if LOG_USB_TO_I2S_SIDE
            printint(0); //printint(total_error);
            printchar(',');
            printintln(g_avg_i2s_send_buffer_level);
#endif

            // This is still WIP so leaving this commented out code here
            //fs_ratio = (unsigned) (((BUFFER_LEVEL_TERM + g_avg_i2s_send_buffer_level) * (unsigned long long)fs_ratio) / BUFFER_LEVEL_TERM);

            /*int guard_level = 100;
            if(g_avg_i2s_send_buffer_level > guard_level)
            {
                int error = g_avg_i2s_send_buffer_level - guard_level;
                fs_ratio = (unsigned) (((BUFFER_LEVEL_TERM + error) * (unsigned long long)fs_ratio) / BUFFER_LEVEL_TERM);
            }
            else if(g_avg_i2s_send_buffer_level < -guard_level)
            {
                int error = g_avg_i2s_send_buffer_level - (-guard_level);
                fs_ratio = (unsigned) (((BUFFER_LEVEL_TERM + error) * (unsigned long long)fs_ratio) / BUFFER_LEVEL_TERM);
            }*/

            usb_to_i2s_rate_ratio = fs_ratio64; // + total_error;

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

#if __xcore__
uint32_t dsp_math_divide_unsigned(uint32_t dividend, uint32_t divisor, uint32_t q_format )
{
    //h and l hold a 64-bit value
    uint32_t h; uint32_t l;
    uint32_t quotient=0, remainder=0;

    // Create long dividend by shift dividend up q_format positions
    h = dividend >> (32-q_format);
    l = dividend << (q_format);

    // Unsigned Long division
    asm("ldivu %0,%1,%2,%3,%4":"=r"(quotient):"r"(remainder),"r"(h),"r"(l),"r"(divisor));

    return quotient;
}
#else //__xcore__
// If we're compiling this for x86 we're probably testing it - let the compiler work out the ASM for this

uint32_t dsp_math_divide_unsigned(uint32_t dividend, uint32_t divisor, uint32_t q_format )
{
    uint64_t h = (uint64_t)dividend << q_format;
    uint64_t quotient = h / divisor;

    return (uint32_t)quotient;
}
#endif //__xcore__

float_s32_t float_div(float_s32_t dividend, float_s32_t divisor)
{
    float_s32_t res;// = float_s32_div(dividend, divisor);

    int dividend_hr;
    int divisor_hr;

    asm( "clz %0, %1" : "=r"(dividend_hr) : "r"(dividend.mant) );
    asm( "clz %0, %1" : "=r"(divisor_hr) : "r"(divisor.mant) );

    int dividend_exp = dividend.exp - dividend_hr;
    int divisor_exp = divisor.exp - divisor_hr;

    uint64_t h_dividend = (uint64_t)((uint32_t)dividend.mant) << (dividend_hr);

    uint32_t h_divisor = ((uint32_t)divisor.mant) << (divisor_hr);

    uint32_t lhs = (h_dividend > h_divisor) ? 31 : 32;

    uint64_t quotient = (h_dividend << lhs) / h_divisor;

    res.exp = dividend_exp - divisor_exp - lhs;

    res.mant = (uint32_t)(quotient) ;
    return res;
}

uint32_t float_div_fixed_output_q_format(float_s32_t dividend, float_s32_t divisor, int32_t output_q_format)
{
    int op_q = -output_q_format;
    float_s32_t res = float_div(dividend, divisor);
    uint32_t quotient;
    if(res.exp < op_q)
    {
        int rsh = op_q - res.exp;
        quotient = ((uint32_t)res.mant >> rsh) + (((uint32_t)res.mant >> (rsh-1)) & 0x1);
    }
    else
    {
        int lsh = res.exp - op_q;
        quotient = (uint32_t)res.mant << lsh;
    }
    return quotient;
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

typedef struct
{
    uint64_t mant;
    int32_t exp;
}float_u64_t;

static float_u64_t float_div_u64(float_s32_t dividend, float_s32_t divisor)
{
    float_u64_t res;

    int dividend_hr;
    int divisor_hr;

    asm( "clz %0, %1" : "=r"(dividend_hr) : "r"(dividend.mant) );
    asm( "clz %0, %1" : "=r"(divisor_hr) : "r"(divisor.mant) );

    int dividend_exp = dividend.exp - dividend_hr;
    int divisor_exp = divisor.exp - divisor_hr;

    uint64_t h_dividend = (uint64_t)((uint32_t)dividend.mant) << (dividend_hr);

    uint32_t h_divisor = ((uint32_t)divisor.mant) << (divisor_hr);

    uint32_t lhs = 32;

    uint64_t quotient = (h_dividend << lhs) / h_divisor;

    res.exp = dividend_exp - divisor_exp - lhs;

    res.mant = quotient ;
    return res;
}

uint64_t float_div_u64_fixed_output_q_format(float_s32_t dividend, float_s32_t divisor, int32_t output_q_format)
{
    int op_q = -output_q_format;
    float_u64_t res = float_div_u64(dividend, divisor);
    uint64_t quotient;
    if(res.exp < op_q)
    {
        int rsh = op_q - res.exp;
        quotient = ((uint64_t)res.mant >> rsh) + (((uint64_t)res.mant >> (rsh-1)) & 0x1);
    }
    else
    {
        int lsh = res.exp - op_q;
        quotient = (uint64_t)res.mant << lsh;
    }
    return quotient;
}
