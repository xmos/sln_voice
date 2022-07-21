// Copyright 2021 - 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#define DEBUG_UNIT ADAPTIVE_USB
#define DEBUG_PRINT_ENABLE_ADAPTIVE_USB 1

// Taken from usb_descriptors.c
#define USB_AUDIO_EP 0x01

#include "adaptive_rate_adjust.h"
#include "adaptive_rate_callback.h"

#include <stdbool.h>
#include <xcore/port.h>
#include <rtos_printf.h>
#include <xscope.h>

#include <xcore/assert.h>
#include <xcore/triggerable.h>
#include <rtos_interrupt.h>

#include "platform/app_pll_ctrl.h"

bool tud_xcore_data_cb(uint32_t cur_time, uint32_t ep_num, uint32_t ep_dir, size_t xfer_len)
{
    if (ep_num == USB_AUDIO_EP &&
        ep_dir == USB_DIR_OUT)
    {
        static uint64_t prev_s;
        uint32_t data_rate = determine_USB_audio_rate(cur_time, xfer_len, ep_dir, true);
        uint64_t s = (uint64_t)data_rate; 
        /* The below manipulations calculate the required f value to scale the nominal app PLL (24.576MHz) by the data rate.
         * The relevant equations are from the XU316 datasheet, and are:
         *
         *                     F + 1 + (f+1 / p+1)      1          1
         * Fpll2 = Fpll2_in *  ------------------- * ------- * --------
         *                             2              R + 1     OD + 1
         * 
         * For given values:
         *  Fpll2_in = 24 (MHz, from oscillator)
         *  F = 408
         *  R = 3
         *  OD = 4
         *  p = 249
         * and expressing Fpll2 as X*s, where X is the nominal frequency and S is the scale applied, we can 
         * rearrange and simplify to give: 
         *
         *      [ f + p + 2     ]           
         *  6 * [ --------- + F ]
         *      [   f + 1       ]   
         *  ---------------------- 
         *  5 * (D + 1) * (R + 1)    = X*s, substituting in values to give
         * 
         *
         *      [ f + 251         ]           
         *  6 * [ --------- + 408 ]
         *      [   250           ]   
         *  ---------------------- 
         *              100         = 24.576 * s, solving for f and simplifying to give
         * 
         * 
         * f = (102400 * s) - 102251, rounded and converted back to an integer from Q31.
         */

        s *= 102400;
        s -= ((uint64_t)102251 << 31);
        s >>= 30;
        s = (s % 2) ? (s >> 1) + 1 : s >> 1;

        if (s != prev_s)
        {
            app_pll_set_numerator((int)s);
            //rtos_printf("New App PLL numerator: %d, data rate: %u\n", (int)s, data_rate);
        }

        prev_s = s;
    }
    return true;
}

bool tud_xcore_sof_cb(uint8_t rhport)
{
    sof_toggle();

    /* False tells TinyUSB to not send the SOF event to the stack */
    return false;
}

void adaptive_rate_adjust_init(chanend_t other_tile_c, xclock_t mclk_clkblk)
{
}
