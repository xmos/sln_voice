// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <stdio.h>
#include <xcore/channel.h>
#include <xcore/port.h>
#include <xcore/parallel.h>
#include <xcore/hwtimer.h>

#include "app_config.h"

#include "xud.h"
#include "xua.h"
#include "xua_commands.h"

// C compatible prototypes for XUA tasks
extern void XUA_Endpoint0(  chanend_t c_ep0_out,
                            chanend_t c_ep0_in,
                            chanend_t c_audioCtrl,
                            unsigned int c_mix_ctl,
                            unsigned int c_clk_ctl,
                            unsigned int c_EANativeTransport_ctrl,
                            unsigned int dfuInterface
);

extern void XUA_Buffer(
            chanend_t c_aud_out,
            chanend_t c_aud_in,
            chanend_t c_aud_fb,
            chanend_t c_sof,
            chanend_t c_aud_ctl,
            port_t p_off_mclk,
            chanend_t c_aud
);


DECLARE_JOB(xud_wrapper, (chanend_t*, const size_t, chanend_t*, const size_t, chanend_t, XUD_EpType*, XUD_EpType*));
void xud_wrapper(chanend_t *chanend_ep_out, const size_t num_ep_out, chanend_t *chanend_ep_in, const size_t num_ep_in, chanend_t c_sof, XUD_EpType *epTypeTableOut, XUD_EpType *epTypeTableIn){
    hwtimer_realloc_xc_timer();
    XUD_Main(chanend_ep_out, num_ep_out, chanend_ep_in, num_ep_in,
             c_sof, epTypeTableOut, epTypeTableIn, 
             XUD_SPEED_HS, XUD_PWR_SELF);
    hwtimer_free_xc_timer();
}

DECLARE_JOB(ep0_wrapper, (chanend_t, chanend_t, chanend_t));
void ep0_wrapper(chanend_t c_ep0_out, chanend_t c_ep0_in, chanend_t c_aud_ctl){
    hwtimer_realloc_xc_timer();
    XUA_Endpoint0(c_ep0_out, c_ep0_in, c_aud_ctl, 0, 0, 0, 0);
    hwtimer_free_xc_timer();
}

DECLARE_JOB(buffer_wrapper, (chanend_t, chanend_t, chanend_t, chanend_t, chanend_t, port_t, chanend_t));
void buffer_wrapper(chanend_t c_ep_aud_out, chanend_t c_ep_aud_in, chanend_t c_ep_fb, chanend_t c_sof, chanend_t c_aud_ctl, port_t p_for_mclk_count, chanend_t c_aud){
    hwtimer_realloc_xc_timer();
    XUA_Buffer(c_ep_aud_out, c_ep_aud_in, c_ep_fb, c_sof, c_aud_ctl, p_for_mclk_count, c_aud);
    hwtimer_free_xc_timer();
}


void xua_wrapper(chanend_t c_aud) {
    printf("xua_wrapper\n");

    const size_t num_ep_out = 2;
    channel_t c_ep_out[num_ep_out];
    chanend_t chanend_ep_out[num_ep_out];

    const size_t num_ep_in = 3;
    channel_t c_ep_in[num_ep_in];
    chanend_t chanend_ep_in[num_ep_in];

    for(int i = 0; i < num_ep_out; i++){
        c_ep_out[i] = chan_alloc();
        chanend_ep_out[i] = c_ep_out[i].end_a;
    }
 
    for(int i = 0; i < num_ep_in; i++){
        c_ep_in[i] = chan_alloc();
        chanend_ep_in[i] = c_ep_in[i].end_a;
    }
 
    channel_t c_sof = chan_alloc();
    channel_t c_aud_ctl = chan_alloc();

    /* Declare enpoint tables */
    XUD_EpType epTypeTableOut[num_ep_out] = {XUD_EPTYPE_CTL | XUD_STATUS_ENABLE, XUD_EPTYPE_ISO};
    XUD_EpType epTypeTableIn[num_ep_in] = {XUD_EPTYPE_CTL | XUD_STATUS_ENABLE, XUD_EPTYPE_ISO, XUD_EPTYPE_ISO};

    /* Declare and enable internal MCLK counting port */
    port_t p_for_mclk_count = PORT_MCLK_COUNT;
    port_enable(p_for_mclk_count);

    /* Connect mclk_count via clock-block to mclk_in pin */
    port_t p_for_mclk_in = USB_MCLK_IN;
    port_enable(p_for_mclk_in);
    xclock_t usb_mclk_in_clk = USB_MCLK_COUNT_CLK_BLK;
    clock_enable(usb_mclk_in_clk);
    clock_set_source_port(usb_mclk_in_clk, p_for_mclk_in);
    port_set_clock(p_for_mclk_count, usb_mclk_in_clk);
    clock_start(usb_mclk_in_clk);

    /* Spawn a total of four threads (buffer is 2) for USB subsystem */
    PAR_JOBS(
        PJOB(xud_wrapper, (chanend_ep_out, num_ep_out, chanend_ep_in, num_ep_in, c_sof.end_a, epTypeTableOut, epTypeTableIn)),
        PJOB(ep0_wrapper, (c_ep_out[0].end_b, c_ep_in[0].end_b, c_aud_ctl.end_a)),
        PJOB(buffer_wrapper, (c_ep_out[1].end_b, c_ep_in[2].end_b, c_ep_in[1].end_b, c_sof.end_b, c_aud_ctl.end_b, p_for_mclk_count, c_aud))
    );
}

/* This function mirrors the API of XUA_Buffer and exchanges samples with USB */
void xua_exchange(chanend_t c_aud, int32_t samples[NUM_USB_CHAN_IN]){
    chanend_out_word(c_aud, 0);
    int isct = chanend_test_control_token_next_byte(c_aud);
    if(isct){
        char ct = chanend_in_control_token(c_aud);
        if(ct == SET_SAMPLE_FREQ)
        {
            chanend_in_word(c_aud); /* Consume sample rate - always one frequency in this app */
        }
        return;
    }

    const unsigned loops = (NUM_USB_CHAN_OUT > 0) ? NUM_USB_CHAN_OUT : 1; 
    for(int i = 0; i < loops; i++)
    {
        chanend_in_word(c_aud); /* Consume USB output samples - none in this app */
    }

    for(int i = 0; i < NUM_USB_CHAN_IN; i++)
    {
        chanend_out_word(c_aud, samples[i]);
    }
}
