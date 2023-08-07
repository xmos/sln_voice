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

#define AVG_I2S_RATE_FILTER_COEFF (0.99)
static uint32_t determine_I2S_rate_simple(
    uint32_t timestamp,
    uint32_t data_length,
    bool update
    )
{
    static uint32_t previous_timestamp = 0;
    static uint32_t prev_nominal_sampling_rate = 0;
    static uint32_t previous_result = 0;
    uint32_t timespan;
    static float_s32_t avg_i2s_rate;

    timespan = timestamp - previous_timestamp;
    previous_timestamp = timestamp;

    if (data_seen == false)
    {
        data_seen = true;
    }

    g_i2s_nominal_sampling_rate = detect_i2s_sampling_rate(i2s_ctx->average_callback_time);
    if(g_i2s_nominal_sampling_rate == 0)
    {
        return 0; // i2s_audio thread ensures we don't do asrc when g_i2s_nominal_sampling_rate is 0
    }
    else if(g_i2s_nominal_sampling_rate != prev_nominal_sampling_rate)
    {
        prev_nominal_sampling_rate = g_i2s_nominal_sampling_rate;
        previous_result = dsp_math_divide_unsigned_64(g_i2s_nominal_sampling_rate, 1000, SAMPLING_RATE_Q_FORMAT); // Samples per ms in SAMPLING_RATE_Q_FORMAT format
        avg_i2s_rate.mant = previous_result;
        avg_i2s_rate.exp = -SAMPLING_RATE_Q_FORMAT;
        return previous_result;
    }

    uint64_t t = (uint64_t)(data_length) * 12500;
    uint32_t samples_per_transaction = dsp_math_divide_unsigned_64(t, (timespan / 8), SAMPLING_RATE_Q_FORMAT); // Samples per millisecond in SAMPLING_RATE_Q_FORMAT
    float_s32_t current_rate;
    current_rate.mant = samples_per_transaction;
    current_rate.exp = -SAMPLING_RATE_Q_FORMAT;

    avg_i2s_rate = float_s32_ema(avg_i2s_rate, current_rate, Q23(AVG_I2S_RATE_FILTER_COEFF));
    avg_i2s_rate = my_ema_calc(avg_i2s_rate, current_rate, Q23(AVG_I2S_RATE_FILTER_COEFF), -SAMPLING_RATE_Q_FORMAT);

    return avg_i2s_rate.mant;

}

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

#define OLD_VAL_WEIGHTING (64)
#define BUFFER_LEVEL_TERM (400000)   //How much to apply the buffer level feedback term (effectively 1/I term)
#define RATE_SERVER_MS  (20)
#define TOTAL_ERROR_AVG_TIME_MS (1000)
#define NUM_ERROR_BUCKETS   (TOTAL_ERROR_AVG_TIME_MS/RATE_SERVER_MS) // Avg over a 500ms period with snapshots every 20ms

#define SAMP_RATE_RATIO_FILTER_COEFF (0.99)

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
    }

    if(error_buckets_full == false)
    {
        error_buckets[rate_server_count] = current_fill_level;
        if(rate_server_count == NUM_ERROR_BUCKETS - 1)
        {
            error_buckets_full = true;
        }
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

void rate_server(void *args)
{
    //unsigned fs_ratio_i2s_to_usb_old = 0;
    //unsigned fs_ratio_usb_to_i2s_old = 0;
    uint32_t prev_ts = get_reference_time();
    uint32_t prev_num_i2s_samples_recvd = i2s_ctx->recv_buffer.total_written;
    uint32_t usb_to_i2s_rate_ratio = 0;
    int32_t usb_buffer_fill_level_from_half;
    static bool reset_buf_level = false;
    usb_to_i2s_rate_info_t usb_rate_info;

    static float_s32_t avg_i2s_to_usb_rate_ratio = {.mant=0x40000000, .exp=-28};
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

        int32_t avg_usb_to_host_buffer_fill_level = get_average_usb_to_host_buf_fill_level(usb_buffer_fill_level_from_half, reset_buf_level);
        if(reset_buf_level)
        {
            reset_buf_level = false;
        }

        // Avg the error

        // Calculate g_i2s_to_usb_rate_ratio only when we're streaming out
        if((i2s_rate != 0) && (usb_rate_info.spkr_itf_open))
        {
            int32_t buffer_level_term = BUFFER_LEVEL_TERM;
            printintln(usb_buffer_fill_level_from_half);
            //printchar(',');
            //printintln(avg_usb_to_host_buffer_fill_level);

            int32_t fs_ratio;
            // fs_ratio_i2s_to_usb_old = g_i2s_to_usb_rate_ratio;
            fs_ratio = dsp_math_divide_unsigned_64(i2s_rate, usb_rate, 28); // Samples per millisecond

            // //printchar(',');
            printhexln(fs_ratio);

            float_s32_t fs_ratio_s32;
            fs_ratio_s32.mant = fs_ratio;
            fs_ratio_s32.exp = -28;
            avg_i2s_to_usb_rate_ratio = my_ema_calc(avg_i2s_to_usb_rate_ratio, fs_ratio_s32, Q30(SAMP_RATE_RATIO_FILTER_COEFF), -28);

            //printchar(',');
            //printhex(avg_i2s_to_usb_rate_ratio.mant);
            if((usb_buffer_fill_level_from_half < -40) || (usb_buffer_fill_level_from_half > 40))
            {
                buffer_level_term = BUFFER_LEVEL_TERM/2;
            }

            //fs_ratio = 0x40000000;
            fs_ratio = (unsigned) (((buffer_level_term + usb_buffer_fill_level_from_half) * (unsigned long long)avg_i2s_to_usb_rate_ratio.mant) / buffer_level_term);



            // fs_ratio = (unsigned) (((unsigned long long)(fs_ratio_i2s_to_usb_old) * OLD_VAL_WEIGHTING + (unsigned long long)(fs_ratio) ) /
            //                 (1 + OLD_VAL_WEIGHTING));

            g_i2s_to_usb_rate_ratio = fs_ratio;

            //printchar(',');
            //printhexln(g_i2s_to_usb_rate_ratio);
        }
        else
        {
            g_i2s_to_usb_rate_ratio = 0;
            reset_buf_level = true;
        }

        // Calculate usb_to_i2s_rate_ratio only when we're recording
        if((i2s_rate != 0) && (usb_rate_info.mic_itf_open))
        {
            //int32_t nom_rate = dsp_math_divide_unsigned_64(g_i2s_nominal_sampling_rate, 1000, SAMPLING_RATE_Q_FORMAT); // Samples per ms in SAMPLING_RATE_Q_FORMAT format
            int32_t fs_ratio;
            //fs_ratio_usb_to_i2s_old = usb_to_i2s_rate_ratio;
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
