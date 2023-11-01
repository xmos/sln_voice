// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <math.h>
#include "usb.h"
#include "usb_rate_calc.h"
#include "NumCpp.hpp"


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
    double nominal_timer_tick_rate = 100e6; // Hz
    int count = 0;
    uint32_t prev_ts = 0;
    bool prev_ts_valid = false;

    if(m_config->usb_timestamps[0].size() != 0)
    {
        uint64_t sample_counter = 0;

        FILE *fp;
        fp = fopen("asrc_input.bin", "wb");

        printf("Schedule USB from logged timestamps\n");
        for (auto ts = m_config->usb_timestamps[0].begin(); ts != m_config->usb_timestamps[0].end(); ts=ts+2)
        {
            //auto sample_space = nc::linspace(sample_counter*(1/m_config->nominal_usb_rate), (sample_counter+48)*(1/m_config->nominal_usb_rate), 48, false);
            // Generate sine tone using the USB rate calculated from SOFs as Fs
            auto sample_space = nc::linspace(sample_counter*(1/m_config->average_usb_rate_from_sofs), (sample_counter+48)*(1/m_config->average_usb_rate_from_sofs), 48, false);
            sample_counter = sample_counter + 48;

            auto samples = 0.5*nc::sin(2*nc::constants::pi * m_config->asrc_input_sine_freq * sample_space);
            //samples.print();

            nc::NdArray<int32_t> samples_q31 = nc::multiply(samples, (double)(1<<31)).astype<int32_t>();
            //printf("%d\n",samples.size());

            int32_t raw_samples[48];


            std::copy(samples_q31.begin(), samples_q31.end(), &raw_samples[0]);
            for(int i=0; i<48; i++)
            {
                m_config->asrc_input_samples[(count*48) + i ] = raw_samples[i];
            }

            if (prev_ts_valid)
            {
                g_avg_usb_rate = determine_USB_audio_rate(*ts, *(ts+1), 0, true);
                //printf("usb_timestamp = %u, g_avg_usb_rate = (0x%x, %d), %.10f\n", *ts, g_avg_usb_rate.mant, g_avg_usb_rate.exp,
                //                                                                ((uint32_t)g_avg_usb_rate.mant)*(pow(2, g_avg_usb_rate.exp))*100000);
                //ts_count += 2;
            }
            count += 1;

            if(count == (m_config->asrc_block_size/48))
            {
                fwrite(m_config->asrc_input_samples, sizeof(int32_t), m_config->asrc_block_size, fp);
                trigger.notify();
                count = 0;
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
                wait_time = ((double)(100000) / nominal_timer_tick_rate) * m_config->nominal_i2s_rate;
            }
            //printf("Waiting for %f us, call_count = %d, ts %lu, prev_ts %lu\n", wait_time, call_count, *ts, prev_ts);
            prev_ts = *ts;
            prev_ts_valid = true;

            wait(wait_time, SC_US);
        }
        printf("End of logged timestamps. Stop simulation\n");
        sc_stop();
    }
    else
    {
        double prev_ts = -1.0;
        FILE *fp;
        fp = fopen("asrc_input.bin", "wb");
        uint64_t sample_counter = 0;

        while(true)
        {
            wait();
            double tstamp = (sc_time_stamp().to_default_time_units()/1000) / m_config->nominal_i2s_rate;
            if(prev_ts >= 0)
            {
                auto sample_space = nc::linspace(prev_ts, tstamp, 48, false);
                //auto sample_space = nc::linspace(sample_counter*(1/m_config->nominal_usb_rate), (sample_counter+48)*(1/m_config->nominal_usb_rate), 48, false);
                sample_counter = sample_counter + 48;

                auto samples = 0.5*nc::sin(2*nc::constants::pi * m_config->asrc_input_sine_freq * sample_space);
                //samples.print();


                nc::NdArray<int32_t> samples_q31 = nc::multiply(samples, (double)(1<<31)).astype<int32_t>();
                //printf("%d\n",samples.size());

                int32_t raw_samples[48];


                std::copy(samples_q31.begin(), samples_q31.end(), &raw_samples[0]);
                for(int i=0; i<48; i++)
                {
                    m_config->asrc_input_samples[(count*48) + i ] = raw_samples[i];
                }

            }
            prev_ts = tstamp;

            count += 1;
            if(count == (m_config->asrc_block_size/48))
            {
                fwrite(m_config->asrc_input_samples, sizeof(int32_t), m_config->asrc_block_size, fp);
                trigger.notify();
                count = 0;
            }
        }
    }
}
