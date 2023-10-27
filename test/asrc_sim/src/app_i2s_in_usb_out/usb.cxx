// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <math.h>
#include "usb.h"
#include "usb_rate_calc.h"
#include "NumCpp.hpp"

extern rate_info_t g_usb_rate_info;

float_s32_t g_avg_usb_rate = {.mant=0, .exp=0};


USB::USB(sc_module_name name, Buffer* buffer, config_t *config)
    : sc_module(name)
    , m_buffer(buffer)
    , m_config(config)
    , clk("clk")
    , trigger("trigger")
{
    if(m_config->usb_timestamps[0].size() != 0)
    {
        SC_THREAD(process);
    }
    else
    {
        SC_THREAD(process); sensitive << clk.pos();
    }

}

void USB::process()
{
    int count = 0;
    uint32_t prev_ts = 0;
    bool prev_ts_valid = false;
    double nominal_timer_tick_rate = 100e6; // Hz

    if(m_config->usb_timestamps[0].size() != 0)
    {
        uint64_t sample_counter = 0;
        FILE *fp;
        fp = fopen("asrc_input.bin", "wb");

        printf("Schedule USB from logged timestamps\n");
        for (auto ts = m_config->usb_timestamps[0].begin(); ts != m_config->usb_timestamps[0].end(); ts=ts+2)
        {
            if (prev_ts_valid)
            {
                g_avg_usb_rate = determine_USB_audio_rate(*ts, *(ts+1), 0, true);
                //printf("usb_timestamp = %u, g_avg_usb_rate = (0x%x, %d), %.10f\n", *ts, g_avg_usb_rate.mant, g_avg_usb_rate.exp,
                //                                                                ((uint32_t)g_avg_usb_rate.mant)*(pow(2, g_avg_usb_rate.exp))*100000);
                //ts_count += 2;
            }

            double wait_time;
            if(prev_ts_valid == true)
            {
                uint32_t ts_diff = *ts - prev_ts;
                //printf("current ts = %u, prev ts = %u, ts diff = %u\n", *ts, prev_ts, ts_diff);
                wait_time = ((double)(ts_diff) / nominal_timer_tick_rate) * m_config->nominal_i2s_rate;
            }
            else
            {
                wait_time = ((double)(100000) / nominal_timer_tick_rate) * m_config->nominal_i2s_rate;  // 1 ms nominally
            }
            //printf("Waiting for %f us, ts %lu, prev_ts %lu\n", wait_time, *ts, prev_ts);
            prev_ts = *ts;
            prev_ts_valid = true;

            m_buffer->read(48);

            wait(wait_time, SC_US);

        }
        printf("End of logged timestamps. Stop simulation\n");
        sc_stop();
    }
    else
    {

        while(true)
        {
            wait();
            m_buffer->read(48);
        }
    }
}
