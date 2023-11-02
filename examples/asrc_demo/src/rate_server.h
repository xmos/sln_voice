// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef RATE_SERVER_H
#define RATE_SERVER_H
#include "xmath/xmath.h"

void rate_server(void *args);

// Getters and setters for various global variables
uint64_t get_i2s_to_usb_rate_ratio();
void set_i2s_to_usb_rate_ratio(uint64_t ratio);
bool get_spkr_itf_close_open_event();
void set_spkr_itf_close_open_event(bool event);

// Rate calculation math functions
uint32_t sum_array(uint32_t * array_to_sum, uint32_t array_length);

// Wrapper functions for calculating i2s send buffer average level
void init_calc_i2s_buffer_level_state(void);
void calc_avg_i2s_send_buffer_level(int32_t current_buffer_level, bool reset);

typedef struct
{
    /* data */
    int64_t buffer_based_correction;
    float_s32_t usb_data_rate;
    int32_t samples_to_host_buf_fill_level;

    bool mic_itf_open;
    bool spkr_itf_open;

}usb_rate_info_t;

typedef struct
{
    /* data */
    uint64_t usb_to_i2s_rate_ratio;
}i2s_to_usb_rate_info_t;

typedef int32_t sw_pll_q24_t; // Type for 15.16 signed fixed point
#define SW_PLL_NUM_FRAC_BITS 24
#define SW_PLL_Q24(val) ((sw_pll_q24_t)((double)val * (1 << SW_PLL_NUM_FRAC_BITS)))

// Kp constants for I2S buffer based control for the USB -> ASRC -> (buffer) -> I2S direction.
#define KP_I2S_BUF_CONTROL_FS48     (SW_PLL_Q24(11.542724608))      // 0.000000043 * (2**28)
#define KP_I2S_BUF_CONTROL_FS96     (SW_PLL_Q24(5.905580032))       // 0.000000022 * (2**28)
#define KP_I2S_BUF_CONTROL_FS192    (SW_PLL_Q24(3.2749125632))      // 0.0000000122 * (2**28)

// Kp constants for USB buffer based control for the I2S -> ASRC -> (buffer) -> USB direction.
#define KP_USB_BUF_CONTROL_FS48     (SW_PLL_Q24(4.563402752))      // 0.000000017 * (2**28)
#define KP_USB_BUF_CONTROL_FS96     (SW_PLL_Q24(9.39524096))       // 0.000000035 * (2**28)
#define KP_USB_BUF_CONTROL_FS192    (SW_PLL_Q24(18.79048192))      // 0.00000007* (2**28)

#endif
