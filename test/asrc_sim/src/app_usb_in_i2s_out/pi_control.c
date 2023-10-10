// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "avg_buffer_level.h"

typedef int32_t sw_pll_q24_t; // Type for 8.24 signed fixed point
#define SW_PLL_NUM_FRAC_BITS 24
#define SW_PLL_Q24(val) ((sw_pll_q24_t)((double)val * (1 << SW_PLL_NUM_FRAC_BITS)))

int32_t calc_accum_error(int32_t error)
{
    #define NUM_BUCKETS (100)
    static int32_t error_buckets[NUM_BUCKETS];
    static int count = 0;
    static bool buckets_full = false;
    if(!buckets_full)
    {
        if(count < NUM_BUCKETS)
        {
            error_buckets[count] = error;
            int64_t accum = 0;
            for(int i=0; i<count; i++)
            {
                accum += error_buckets[i];
            }
            count += 1;
            if(count == NUM_BUCKETS)
            {
                buckets_full = true;
            }
            return accum;
        }
    }
    else
    {
        int oldest_bucket = count % NUM_BUCKETS;
         error_buckets[oldest_bucket] = error;
         count += 1;
        int64_t accum = 0;
        for(int i=0; i<NUM_BUCKETS; i++)
        {
            accum += error_buckets[i];
        }
        return accum;
    }
    return 0;
}

static inline sw_pll_q24_t get_Kp_for_usb_buffer_control(int32_t nominal_i2s_rate)
{
    sw_pll_q24_t Kp = 0;
    if(((int)nominal_i2s_rate == (int)44100) || ((int)nominal_i2s_rate == (int)48000))
    {
        Kp = SW_PLL_Q24(11.542724608); // 0.000000043 * (2**28)
    }
    else if(((int)nominal_i2s_rate == (int)88200) || ((int)nominal_i2s_rate == (int)96000))
    {
        Kp = SW_PLL_Q24(5.905580032); // 0.000000022 * (2**28)
    }
    else if(((int)nominal_i2s_rate == (int)176400) || ((int)nominal_i2s_rate == (int)192000))
    {
        Kp = SW_PLL_Q24(3.2749125632); // 0.0000000122 * (2**28)
    }
    return Kp;
}

uint64_t pi_control(int32_t nominal_i2s_rate, buffer_calc_state_t *buf_state)
{
    sw_pll_q24_t Kp = get_Kp_for_usb_buffer_control(nominal_i2s_rate);
    int64_t max_allowed_correction = (int64_t)1500 << 32;
    int64_t total_error = 0;

    if(buf_state->flag_stable_avg == true)
    {
        int64_t error_p = ((int64_t)Kp * (int64_t)(buf_state->avg_buffer_level - buf_state->stable_avg_level));

        total_error = (int64_t)(error_p << 8);
        if(total_error > max_allowed_correction)
        {
            total_error = max_allowed_correction;
        }
        else if(total_error < -(max_allowed_correction))
        {
            total_error = -(max_allowed_correction);
        }
    }

    return total_error;

}
