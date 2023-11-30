// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "pi_control.h"


typedef int32_t sw_pll_q24_t; // Type for 8.24 signed fixed point
#define SW_PLL_NUM_FRAC_BITS 24
#define SW_PLL_Q24(val) ((sw_pll_q24_t)((float)val * (1 << SW_PLL_NUM_FRAC_BITS)))


static inline sw_pll_q24_t get_Kp_for_usb_buffer_control(int32_t nominal_i2s_rate)
{
    sw_pll_q24_t Kp = 0;
    if(((int)nominal_i2s_rate == (int)44100) || ((int)nominal_i2s_rate == (int)48000))
    {
        Kp = SW_PLL_Q24(4.563402752); // 0.000000017 * (2**28)
    }
    else if(((int)nominal_i2s_rate == (int)88200) || ((int)nominal_i2s_rate == (int)96000))
    {
        Kp = SW_PLL_Q24(9.39524096); // 0.000000035 * (2**28)
    }
    else if(((int)nominal_i2s_rate == (int)176400) || ((int)nominal_i2s_rate == (int)192000))
    {
        Kp = SW_PLL_Q24(18.79048192); // 0.00000007* (2**28)
    }
    return Kp;
}

uint64_t calc_usb_buffer_based_correction(int32_t nominal_i2s_rate, buffer_calc_state_t *long_term_buf_state, buffer_calc_state_t *short_term_buf_state)
{
    sw_pll_q24_t Kp = get_Kp_for_usb_buffer_control(nominal_i2s_rate);
    int64_t max_allowed_correction = (int64_t)1500 << 32;
    int64_t total_error = 0;

    // Correct based on short term average only when creeping outside the guard band
    if(short_term_buf_state->flag_stable_avg == true)
    {
        if(short_term_buf_state->avg_buffer_level > 300)
        {
            total_error = max_allowed_correction;
            return total_error;
        }
        else if(short_term_buf_state->avg_buffer_level < -300)
        {
            total_error = -(max_allowed_correction);
            return total_error;
        }
    }
    if(long_term_buf_state->flag_stable_avg == true)
    {
        int64_t error_p = ((int64_t)Kp * (int64_t)(long_term_buf_state->avg_buffer_level - long_term_buf_state->stable_avg_level));

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
