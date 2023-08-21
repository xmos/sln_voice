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

#define REF_CLOCK_TICKS_PER_SECOND 100000000


extern uint32_t dsp_math_divide_unsigned_64(uint64_t dividend, uint32_t divisor, uint32_t q_format );
extern uint32_t sum_array(uint32_t * array_to_sum, uint32_t array_length);
extern uint32_t dsp_math_divide_unsigned(uint32_t dividend, uint32_t divisor, uint32_t q_format );
float_s32_t float_div(float_s32_t dividend, float_s32_t divisor);
uint32_t float_div_fixed_output_q_format(float_s32_t dividend, float_s32_t divisor, int32_t output_q_format);

// Global variables shared with i2s_audio.c
uint32_t g_i2s_to_usb_rate_ratio = 0;
uint32_t g_i2s_nominal_sampling_rate = 0;

extern int32_t g_avg_i2s_send_buffer_level;
extern int32_t g_prev_avg_i2s_send_buffer_level;

bool g_spkr_itf_close_to_open = false;

float_s32_t my_ema_calc(float_s32_t x, float_s32_t y, uint32_t alpha_q30, int32_t output_exp)
{
    float_s32_t temp = float_s32_ema(x, y, alpha_q30);

    if(temp.exp < output_exp)
    {
        temp.mant = (temp.mant >> (output_exp - temp.exp));
    }
    else
    {
        temp.mant = (temp.mant << (temp.exp - output_exp));
    }
    temp.exp = output_exp;
    return temp;
}

uint32_t my_ema_calc_custom(uint32_t x, uint32_t y, int input_exp, uint32_t alpha_q31, int32_t output_exp)
{
    uint32_t one_minus_alpha = 0x7fffffff - alpha_q31;
    uint64_t m1 = (uint64_t)x * alpha_q31;
    uint64_t m2 = (uint64_t)y * one_minus_alpha;
    uint64_t m3 = m1 + m2;
    uint32_t m_exp = input_exp + (- 31);
    int rsh = output_exp - m_exp;

    uint32_t temp = (uint32_t)(m3 >> rsh);
    return temp;
}

#define TOTAL_STORED_AVG_I2S_RATE (16)
static float_s32_t determine_avg_I2S_rate_from_driver(
    uint32_t timespan,
    uint32_t num_samples,
    bool update
    )
{
    static uint32_t data_lengths[TOTAL_STORED_AVG_I2S_RATE];
    static uint32_t time_buckets[TOTAL_STORED_AVG_I2S_RATE];
    static uint32_t current_data_bucket_size;
    static bool buckets_full;
    static uint32_t times_overflowed;
    static float_s32_t previous_result = {.mant = 0, .exp = 0};
    static uint32_t prev_nominal_sampling_rate = 0;
    static uint32_t counter = 0;
    static uint32_t timespan_current_bucket = 0;

    g_i2s_nominal_sampling_rate = i2s_ctx->i2s_nominal_sampling_rate;
    if(g_i2s_nominal_sampling_rate == 0)
    {
        float_s32_t t = {.mant=0, .exp=0};
        return t; // i2s_audio thread ensures we don't do asrc when g_i2s_nominal_sampling_rate is 0
    }
    else if(g_i2s_nominal_sampling_rate != prev_nominal_sampling_rate)
    {
        rtos_printf("determine_avg_I2S_rate_from_driver() SR change detected\n");
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
        prev_nominal_sampling_rate = g_i2s_nominal_sampling_rate;
        float_s32_t a = {.mant=g_i2s_nominal_sampling_rate, .exp=0};
        float_s32_t b = {.mant=REF_CLOCK_TICKS_PER_SECOND, .exp=0};

        float_s32_t rate = float_div(a, b); // Samples per ms in SAMPLING_RATE_Q_FORMAT format
        previous_result = rate;

        return previous_result;
    }
    else if(timespan == 0)
    {
        float_s32_t a = {.mant=prev_nominal_sampling_rate, .exp=0};
        float_s32_t b = {.mant=REF_CLOCK_TICKS_PER_SECOND, .exp=0};

        float_s32_t rate = float_div(a, b); // Samples per ms in SAMPLING_RATE_Q_FORMAT format
        previous_result = rate;

        return previous_result;

    }

    counter += 1;

    if (update)
    {
        current_data_bucket_size += num_samples;
        timespan_current_bucket += timespan;
    }

    uint32_t total_data_intermed = current_data_bucket_size + sum_array(data_lengths, TOTAL_STORED_AVG_I2S_RATE);
    uint32_t total_timespan = timespan_current_bucket + sum_array(time_buckets, TOTAL_STORED_AVG_I2S_RATE);

    float_s32_t data_per_sample = float_div((float_s32_t){total_data_intermed, 0}, (float_s32_t){total_timespan, 0});

    /*float_s32_t data_per_sample1 = float_div((float_s32_t){num_samples, 0}, (float_s32_t){timespan, 0});

    printuint(data_per_sample.mant);
    printchar(',');
    printint(data_per_sample.exp);
    printchar(',');
    printuint(data_per_sample1.mant);
    printchar(',');
    printintln(data_per_sample1.exp);*/

    float_s32_t result = data_per_sample;

    if (update && (counter >= 16))
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

#if 0
#define OLD_VAL_WEIGHTING (64)
#define NUM_ERROR_BUCKETS   (2048)

#define SAMP_RATE_RATIO_FILTER_COEFF (0.95)

static int32_t get_average_usb_to_host_buf_fill_level(int32_t current_fill_level, bool reset)
{
    static int32_t error_buckets[NUM_ERROR_BUCKETS] = {0};
    static bool error_buckets_full = false;
    static uint32_t rate_server_count = 0;

    if(reset)
    {
        rate_server_count = 0;
        error_buckets_full = false;
        memset(error_buckets, 0, sizeof(error_buckets));
        return 0;
    }

    if(error_buckets_full == false)
    {
        error_buckets[rate_server_count] = current_fill_level;
        if(rate_server_count == NUM_ERROR_BUCKETS - 1)
        {
            error_buckets_full = true;
        }
        rate_server_count += 1;
        return 0;
    }
    else
    {
        uint32_t oldest = rate_server_count % NUM_ERROR_BUCKETS;
        error_buckets[oldest] = current_fill_level;
    }
    int32_t total_error = 0;
    for(int i=0; i< NUM_ERROR_BUCKETS; i++)
    {
        total_error = total_error + error_buckets[i];
    }
    int32_t avg_fill_level = total_error / NUM_ERROR_BUCKETS;

    rate_server_count += 1;
    return avg_fill_level;
}

#define NUM_RATE_AVG_BUCKETS   (512) // Avg over a 500ms period with snapshots every 20ms
static int32_t get_average_rate_ratio(uint32_t current_ratio, bool reset)
{
    static uint32_t buckets[NUM_RATE_AVG_BUCKETS] = {0};
    static bool buckets_full = false;
    static uint32_t rate_server_count = 0;

    if(reset)
    {
        rate_server_count = 0;
        buckets_full = false;
        memset(buckets, 0, sizeof(buckets));
        return 0;
    }

    if(buckets_full == false)
    {
        buckets[rate_server_count] = current_ratio;
        if(rate_server_count == NUM_RATE_AVG_BUCKETS - 1)
        {
            buckets_full = true;
        }
        rate_server_count += 1;
        return 0;
    }
    else
    {
        uint32_t oldest = rate_server_count % NUM_RATE_AVG_BUCKETS;
        buckets[oldest] = current_ratio;
    }
    uint64_t total = 0;
    for(int i=0; i< NUM_RATE_AVG_BUCKETS; i++)
    {
        total = total + (uint64_t)buckets[i];
    }
    uint32_t avg = total / NUM_RATE_AVG_BUCKETS;

    rate_server_count += 1;

    return avg;
}
#endif

typedef int32_t sw_pll_15q16_t; // Type for 15.16 signed fixed point
#define SW_PLL_NUM_FRAC_BITS 16
#define SW_PLL_15Q16(val) ((sw_pll_15q16_t)((float)val * (1 << SW_PLL_NUM_FRAC_BITS)))

#define BUFFER_LEVEL_TERM (400000)   //How much to apply the buffer level feedback term (effectively 1/I term)

void rate_server(void *args)
{
    uint32_t usb_to_i2s_rate_ratio = 0;
    int32_t usb_buffer_fill_level_from_half;
    static bool reset_buf_level = false;
    static bool prev_spkr_itf_open = false;
    usb_to_i2s_rate_info_t usb_rate_info;
    i2s_to_usb_rate_info_t i2s_rate_info;

    const sw_pll_15q16_t Ki = SW_PLL_15Q16(0.2);
    const sw_pll_15q16_t Kd = SW_PLL_15Q16(0.25634765625);

    while(g_i2s_nominal_sampling_rate == 0)
    {
        // Make sure we establish I2S rate before starting, so that the i2s_audio_recv_task() can get going and start sending
        // frames to the other tile, which will prompt the periodic rate monitoring
        vTaskDelay(pdMS_TO_TICKS(1));
        float_s32_t avg_rate = determine_avg_I2S_rate_from_driver(i2s_ctx->write_256samples_time, 3840, true);
        (void)avg_rate;
    }
    rtos_printf("ready to start! I2S rate %d\n", g_i2s_nominal_sampling_rate);

    for(;;)
    {
        // Get USB rate and buffer information from the other tile
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
            g_spkr_itf_close_to_open = true;
        }
        prev_spkr_itf_open = usb_rate_info.spkr_itf_open;

        // Compute I2S rate
        float_s32_t avg_rate = determine_avg_I2S_rate_from_driver(i2s_ctx->write_256samples_time, 3840, true);
        float_s32_t i2s_rate = avg_rate;

        usb_buffer_fill_level_from_half = usb_rate_info.samples_to_host_buf_fill_level / 8;

        // Calculate g_i2s_to_usb_rate_ratio only when the host is recording data from the device
        if((i2s_rate.mant != 0) && (usb_rate_info.mic_itf_open))
        {
            int32_t buffer_level_term = BUFFER_LEVEL_TERM;
            //printint(usb_buffer_fill_level_from_half);
            //printchar(',');

            int32_t fs_ratio = float_div_fixed_output_q_format(i2s_rate, usb_rate, 28);



            /*printchar(',');
            printuint(i2s_rate.mant);
            printchar(',');
            printint(i2s_rate.exp);
            printchar(',');
            printuint(usb_rate.mant);
            printchar(',');
            printint(usb_rate.exp);*/
            //printchar(',');

            //printintln(fs_ratio);

            int guard_level = 100;
            if(usb_buffer_fill_level_from_half > guard_level)
            {
                int error = usb_buffer_fill_level_from_half - guard_level;
                fs_ratio = (unsigned) (((buffer_level_term + error) * (unsigned long long)fs_ratio) / buffer_level_term);
            }
            else if(usb_buffer_fill_level_from_half < -guard_level)
            {
                int error = usb_buffer_fill_level_from_half - (-guard_level);
                fs_ratio = (unsigned) (((buffer_level_term + error) * (unsigned long long)fs_ratio) / buffer_level_term);
            }

            //fs_ratio = (unsigned) (((buffer_level_term + usb_buffer_fill_level_from_half) * (unsigned long long)fs_ratio) / buffer_level_term);


            g_i2s_to_usb_rate_ratio = fs_ratio;

            if(reset_buf_level)
            {
                reset_buf_level = false;
            }
            //printchar(',');
            //printhexln(g_i2s_to_usb_rate_ratio);
        }
        else
        {
            g_i2s_to_usb_rate_ratio = 0;
            reset_buf_level = true;
        }

        // Calculate usb_to_i2s_rate_ratio only when the host is playing data to the device
        if((i2s_rate.mant != 0) && (usb_rate_info.spkr_itf_open))
        {
            //int32_t nom_rate = dsp_math_divide_unsigned_64(g_i2s_nominal_sampling_rate, 1000, SAMPLING_RATE_Q_FORMAT); // Samples per ms in SAMPLING_RATE_Q_FORMAT format
            //fs_ratio_usb_to_i2s_old = usb_to_i2s_rate_ratio;
            int32_t fs_ratio = float_div_fixed_output_q_format(usb_rate, i2s_rate, 28);

            int64_t error_d = ((int64_t)Kd * (int64_t)(g_avg_i2s_send_buffer_level - g_prev_avg_i2s_send_buffer_level));
            int64_t error_i = ((int64_t)Ki * (int64_t)g_avg_i2s_send_buffer_level);

            int32_t total_error = (int32_t)((error_d + error_i) >> SW_PLL_NUM_FRAC_BITS);
            (void)total_error;

            //printint(total_error);
            //printchar(',');
            //printint(g_avg_i2s_send_buffer_level);
            //printchar(',');


            //fs_ratio = (unsigned) (((BUFFER_LEVEL_TERM + g_avg_i2s_send_buffer_level) * (unsigned long long)fs_ratio) / BUFFER_LEVEL_TERM);

            /*fs_ratio = (unsigned) (((unsigned long long)(fs_ratio_usb_to_i2s_old) * OLD_VAL_WEIGHTING + (unsigned long long)(fs_ratio) ) /
                            (1 + OLD_VAL_WEIGHTING));*/


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
            //printchar(',');
            //printhexln(usb_to_i2s_rate_ratio);
            //printf("usb_to_i2s_rate_ratio = %f\n", (float)usb_to_i2s_rate_ratio/(1<<28));
            usb_to_i2s_rate_ratio = fs_ratio/* + total_error*/;
            //printintln(usb_to_i2s_rate_ratio);

        }
        else
        {
            usb_to_i2s_rate_ratio = 0;
        }

        i2s_rate_info.nominal_i2s_freq = g_i2s_nominal_sampling_rate;
        i2s_rate_info.usb_to_i2s_rate_ratio = usb_to_i2s_rate_ratio ;

        rtos_intertile_tx(
            intertile_ctx,
            appconfUSB_RATE_NOTIFY_PORT,
            &i2s_rate_info,
            sizeof(i2s_rate_info));

        // Get the I2S rate

    }
}
