// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include "i2s.h"
#include "asrc.h"
#include "NumCpp.hpp"


I2S::I2S(sc_module_name name, Buffer* buffer, config_t *config)
    : sc_module(name)
    , m_buffer(buffer)
    , m_config(config)
    , clk("clk")
    , trigger("trigger")
{
    SC_THREAD(process); sensitive << clk.pos();
}

void I2S::process()
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
            auto sample_space = nc::linspace(prev_ts, tstamp, m_config->asrc_block_size, false);

            // Warp Fs by nominal_output_rate/actual_output_rate so we can have the FFT peak of the output in one bin and not spread all over the spectrum.
            // Note that when m_config->usb_timestamps[0].size() != 0, we use the average USB rate calculated from SOFs by averaging over the entire duration
            // for which the SOFs were logged. This is an approximation at best so its very difficult to get a good FFT peak in the freq domain for the ASRC output
            // which affects the SNR calculation.
            double fs;
            if(m_config->usb_timestamps[0].size() == 0)
            {
                fs = (double)m_config->nominal_i2s_rate * (m_config->nominal_usb_rate / m_config->actual_usb_rate);
            }
            else
            {
                fs = (double)m_config->nominal_i2s_rate * (m_config->nominal_usb_rate / m_config->average_usb_rate_from_sofs);
            }
            sample_space = nc::linspace(sample_counter*(1/fs), (sample_counter + m_config->asrc_block_size)*(1/fs), m_config->asrc_block_size, false);
            sample_counter = sample_counter + m_config->asrc_block_size;

            auto samples = 0.5*nc::sin(2*nc::constants::pi * m_config->asrc_input_sine_freq * sample_space);
            //samples.print();


            nc::NdArray<int32_t> samples_q31 = nc::multiply(samples, (double)(1<<31)).astype<int32_t>();
            //printf("%d\n",samples.size());

            std::copy(samples_q31.begin(), samples_q31.end(), &m_config->asrc_input_samples[0]);

        }
        prev_ts = tstamp;

        fwrite(m_config->asrc_input_samples, sizeof(int32_t), m_config->asrc_block_size, fp);
        trigger.notify();
    }
}
