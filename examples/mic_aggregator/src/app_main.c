// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <stdio.h>
#include <limits.h>

#include <xcore/channel.h>
#include <xcore/channel_streaming.h>
#include <xcore/parallel.h>
#include <xcore/select.h>
#include <xcore/hwtimer.h>
#include <print.h>

#include "app_main.h"
#include "mic_array.h"
#include "device_pll_ctrl.h"
#include "mic_array_wrapper.h"
#include "tdm_slave_wrapper.h"
#include "tdm_master_simple.h"
#include "i2c_control.h"

#include "xua_wrapper.h"
#include "xua_conf.h"


DECLARE_JOB(pdm_mic_16, (chanend_t));
void pdm_mic_16(chanend_t c_mic_array) {
    printf("pdm_mic_16 running: %d threads total\n", MIC_ARRAY_PDM_RX_OWN_THREAD + MIC_ARRAY_NUM_DECIMATOR_TASKS);

    app_mic_array_init();
    // app_mic_array_assertion_disable();
    app_mic_array_assertion_enable();   // Inform if timing is not met
    app_mic_array_task(c_mic_array);
}

DECLARE_JOB(pdm_mic_16_front_end, (void));
void pdm_mic_16_front_end(void) {
    printf("pdm_mic_16_front_end\n");

    if(MIC_ARRAY_PDM_RX_OWN_THREAD){
        app_pdm_rx_task();
    }
}

static inline int32_t scalar_gain(int32_t samp, int32_t gain){
    int64_t accum = (int64_t)samp * (int32_t)gain;
    accum = accum > INT_MAX ? INT_MAX : accum;
    accum = accum < INT_MIN ? INT_MIN : accum;

    return (int32_t)accum;
}


DECLARE_JOB(hub, (chanend_t, chanend_t, chanend_t, audio_frame_t **));
void hub(chanend_t c_mic_array, chanend_t c_i2c_reg, chanend_t c_aud, audio_frame_t **read_buffer_ptr) {
    printf("hub\n");

    unsigned write_buffer_idx = 0;
    audio_frame_t audio_frames[NUM_AUDIO_BUFFERS] = {{{{0}}}};

    uint16_t gains[MIC_ARRAY_CONFIG_MIC_COUNT] = {MIC_GAIN_INIT, MIC_GAIN_INIT, MIC_GAIN_INIT, MIC_GAIN_INIT,
                                                  MIC_GAIN_INIT, MIC_GAIN_INIT, MIC_GAIN_INIT, MIC_GAIN_INIT,
                                                  MIC_GAIN_INIT, MIC_GAIN_INIT, MIC_GAIN_INIT, MIC_GAIN_INIT,
                                                  MIC_GAIN_INIT, MIC_GAIN_INIT, MIC_GAIN_INIT, MIC_GAIN_INIT};  
    while(1){
        ma_frame_rx((int32_t*)&audio_frames[write_buffer_idx], c_mic_array, MIC_ARRAY_CONFIG_MIC_COUNT, MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME);

        // Apply gain
        for(int ch = 0; ch < MIC_ARRAY_CONFIG_MIC_COUNT; ch++){
            audio_frames[write_buffer_idx].data[ch][0] = scalar_gain(audio_frames[write_buffer_idx].data[ch][0], gains[ch]);
        }
#if CONFIG_USB
        xua_exchange(c_aud, &audio_frames[write_buffer_idx].data[0][0]);
#endif
        *read_buffer_ptr = &audio_frames[write_buffer_idx];  // update read buffer for TDM

        write_buffer_idx++;
        if(write_buffer_idx == NUM_AUDIO_BUFFERS){
            write_buffer_idx = 0;
        }

        // Handle any control updates from the host
        // Non-blocking channel read on c_i2c_reg
        SELECT_RES(
            CASE_THEN(c_i2c_reg, i2c_register_write),
            DEFAULT_THEN(drop_through)
        )
        {
            i2c_register_write:
            {
                uint8_t channel = s_chan_in_byte(c_i2c_reg);
                uint8_t data_h = s_chan_in_byte(c_i2c_reg);
                uint8_t data_l = s_chan_in_byte(c_i2c_reg);

                int32_t gain = U16_FROM_BYTES(data_h, data_l);
                gains[channel] = gain;
            }
            break;

            drop_through:
            {
                // Do nothing & fall-through
            }
            break;
        }
        // There are currently around 1600 ticks (16us) of slack at the end of this loop in TDM mode
    }
}



///////// Tile main functions where we par off the threads ///////////

void main_tile_0(chanend_t c_cross_tile[2]){
    PAR_JOBS(
        PJOB(pdm_mic_16, (c_cross_tile[0])), // Note spawns MIC_ARRAY_NUM_DECIMATOR_TASKS threads
        PJOB(pdm_mic_16_front_end, ())
#if CONFIG_TDM
        ,PJOB(i2c_control, (c_cross_tile[1]))
#endif
    );
}

void main_tile_1(chanend_t c_cross_tile[2]){
    // Pointer to pointer for sharing the mic_array read_buffer last write
    audio_frame_t *read_buffer = NULL;
    audio_frame_t **read_buffer_ptr = &read_buffer;

    // Enable and setup the 24.576MHz APP PLL which is used as BCLK for TDM or MCLK for USB and prescaled as PDM clock for both
    port_t p_app_pll_out = MIC_ARRAY_CONFIG_PORT_MCLK;
    port_enable(p_app_pll_out);
    set_pad_drive_strength(p_app_pll_out, DRIVE_12MA);
    device_pll_init();

    channel_t c_aud = chan_alloc();

    PAR_JOBS(
        PJOB(hub, (c_cross_tile[0], c_cross_tile[1], c_aud.end_b, read_buffer_ptr)),
#if CONFIG_TDM
        PJOB(tdm16_slave, (read_buffer_ptr)),
        PJOB(tdm16_master_simple, ()),
        PJOB(tdm_master_monitor, ()) // Temp monitor for checking reception of TDM frames. Separate task so non-intrusive
#else
        PJOB(xua_wrapper, (c_aud.end_a)) // This spawns 4 tasks
#endif
    );
}