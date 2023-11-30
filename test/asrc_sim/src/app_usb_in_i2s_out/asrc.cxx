// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include "asrc.h"
#include "ASRC_wrapper.h"
#include "usb_rate_calc.h"
#include "pi_control.h"
#include "avg_buffer_level.h"

extern float_s32_t g_avg_usb_rate;
float_s32_t g_avg_i2s_rate;


ASRC::ASRC(sc_module_name name, uint32_t fs_in, uint32_t fs_out, uint32_t block_size, double actual_rate_ratio, Buffer* buffer, sc_event &trigger, config_t *config)
    : sc_module(name)
    , m_buffer(buffer)
    , m_config(config)
    , m_trigger(trigger)
{
    uint32_t rand_seed[MAX_ASRC_N_IO_CHANNELS] = {0};

    m_nominal_rate_ratio = wrapper_asrc_init(&m_profile_info_ptr, fs_in, fs_out, block_size, 1, 1, ASRC_DITHER_OFF, rand_seed);
    m_nominal_rate_ratio_f = (double)m_nominal_rate_ratio/(uint64_t)((uint64_t)1<<(28+32));
    m_block_size = block_size;

    printf("nominal_rate_ratio = %f, block_size = %u\n", m_nominal_rate_ratio_f, m_block_size);

    m_actual_rate_ratio_f = actual_rate_ratio;// + 1e-8;    // Optionally, add an extra drift
    m_actual_rate_ratio = uint64_t(m_actual_rate_ratio_f * ((uint64_t)1 << (28+32)));

    g_avg_i2s_rate = float_div((float_s32_t){(int32_t)m_config->nominal_i2s_rate, 0}, (float_s32_t){100000000, 0});

    printf("I2S rate = (0x%x, %d), %.10f\n", g_avg_i2s_rate.mant, g_avg_i2s_rate.exp, ((uint32_t)g_avg_i2s_rate.mant)*pow(2, g_avg_i2s_rate.exp));

    SC_THREAD(process); sensitive << trigger;
}

void ASRC::process()
{
    int32_t input[m_block_size];
    int32_t output[int(2*(m_block_size/m_nominal_rate_ratio_f)) * 2];
    uint32_t buffer_writes_count = 0;
    buffer_calc_state_t buf_state;

    uint64_t rate_ratio;
    if(m_config->usb_timestamps[0].size() != 0)
    {
        rate_ratio = m_nominal_rate_ratio;
    }
    else
    {
        rate_ratio = m_actual_rate_ratio;
    }

    init_calc_buffer_level_state(&buf_state, 10, 8);

    FILE *fp;
    fp = fopen("asrc_output.bin", "wb");
    while(true)
    {
        wait();
        uint32_t num_out_samples = wrapper_asrc_process(&m_config->asrc_input_samples[0], &output[0], rate_ratio);

        fwrite(&output[0], sizeof(int32_t), num_out_samples, fp);

        //unsigned int asrc_delay = 60 + (rand() % 20);
        //wait(asrc_delay, SC_US);
        m_buffer->write(num_out_samples);

        calc_avg_buffer_level(&buf_state, m_buffer->fill_level(), false);

        // After 16 writes
        buffer_writes_count += 1;

        if(buffer_writes_count == 16)
        {
            int64_t error;
            if(m_config->usb_timestamps[0].size() != 0)
            {
                rate_ratio = float_div_u64_fixed_output_q_format(g_avg_usb_rate, g_avg_i2s_rate, 28+32);
                error = pi_control(m_config->nominal_i2s_rate, &buf_state);
                // Uncomment to apply a fixed correction instead of the pi_control() code.
                //double correction = 0.000000043;
                //uint64_t correction_i = (uint64_t)(correction * ((uint64_t)1 << (28+32)));
                //rate_ratio = rate_ratio - correction_i;
                rate_ratio = rate_ratio + error;
            }
            else
            {
                error = pi_control(m_config->nominal_i2s_rate, &buf_state);
                rate_ratio = m_actual_rate_ratio + error;
            }

            buffer_writes_count = 0;

            if(buf_state.flag_stable_avg)
            {
                printf("%d,%d\n",m_buffer->fill_level(),buf_state.avg_buffer_level);
            }
        }

    }
}
