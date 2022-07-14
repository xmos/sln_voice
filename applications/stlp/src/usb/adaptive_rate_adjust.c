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

#if XK_VOICE_L71
#define PORT_MCLK       PORT_MCLK_IN_OUT
#elif XCOREAI_EXPLORER
#define PORT_MCLK       PORT_MCLK_IN
#elif OSPREY_BOARD
#define PORT_MCLK       PORT_MCLK_IN
#else
#ifndef PORT_MCLK
#define PORT_MCLK       0
#endif
#endif

static chanend_t sof_t1_isr_c;

bool tud_xcore_data_cb(uint32_t cur_time, uint32_t ep_num, uint32_t ep_dir, size_t xfer_len)
{
    if (ep_num == USB_AUDIO_EP &&
        ep_dir == USB_DIR_OUT)
    {
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
        s -= (102251 << 31);
        s >>= 30;
        s = (s % 2) ? (s >> 1) + 1 : s >> 1;

        app_pll_set_numerator((int)s);
    }
    return true;
}

bool tud_xcore_sof_cb(uint8_t rhport)
{
#if XCOREAI_EXPLORER
    sof_toggle();
#else
    chanend_out_end_token(sof_t1_isr_c);
#endif

    /* False tells TinyUSB to not send the SOF event to the stack */
    return false;
}

DEFINE_RTOS_INTERRUPT_CALLBACK(sof_t1_isr, arg)
{
    (void) arg;

    chanend_check_end_token(sof_t1_isr_c);
    sof_toggle();
}

#if XK_VOICE_L71 || OSPREY_BOARD
static void sof_intertile_init(chanend_t other_tile_c)
{
    sof_t1_isr_c = chanend_alloc();
    xassert(sof_t1_isr_c != 0);

#if ON_TILE(1)
    chanend_out_word(other_tile_c, sof_t1_isr_c);
    chanend_out_end_token(other_tile_c);
#endif
#if ON_TILE(0)
    chanend_set_dest(sof_t1_isr_c, chanend_in_word(other_tile_c));
    chanend_check_end_token(other_tile_c);
#endif

#if ON_TILE(1)
    /*
     * TODO: Move this to adaptive_rate_adjust_start() perhaps,
     * and then call it from platform_start().
     * It could then support enabling the ISR on a specified core.
     * ATM, the ISR will run on whatever core is running prior to
     * the RTOS starting, which isn't guaranteed to be anything,
     * though seems to always be RTOS core 0. This is fine, but
     * the tick interrupt can interfere, and if it does happen to
     * end up on the mic or i2s cores, that might be bad.
     */
    triggerable_setup_interrupt_callback(sof_t1_isr_c,
                                         NULL,
                                         RTOS_INTERRUPT_CALLBACK(sof_t1_isr));
    triggerable_enable_trigger(sof_t1_isr_c);
#endif
}
#endif

void adaptive_rate_adjust_init(chanend_t other_tile_c, xclock_t mclk_clkblk)
{
#if (XCOREAI_EXPLORER && ON_TILE(0)) || ((XK_VOICE_L71 || OSPREY_BOARD) && ON_TILE(1))
    /*
     * Configure the MCLK input port on the tile that
     * will run sof_cb() and count its clock cycles.
     *
     * On the Explorer board the appPLL/MCLK output from
     * tile 1 is wired over to tile 0. On the Osprey and
     * 3610 boards it is not.
     *
     * It is set up to clock itself. This allows GETTS to
     * be called on it to count its clock cycles. This
     * count is used to adjust its frequency to match the
     * USB host.
     */
    port_enable(PORT_MCLK);
    clock_enable(mclk_clkblk);
    clock_set_source_port(mclk_clkblk, PORT_MCLK);
    port_set_clock(PORT_MCLK, mclk_clkblk);
    clock_start(mclk_clkblk);
#endif
#if XK_VOICE_L71 || OSPREY_BOARD
    /*
     * On the Osprey and 3610 boards an additional intertile
     * channel and ISR must be set up in order to run the
     * SOF ISR on tile 1, since MCLK is not wired over to
     * tile 0.
     */
    sof_intertile_init(other_tile_c);
#endif
}
