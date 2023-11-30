// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include "asrc.h"
#include "ASRC_wrapper.h"
#include "usb_rate_calc.h"
#include "pi_control.h"
#include "usb_rate_calc.h"
#include "avg_buffer_level.h"

extern float_s32_t g_avg_usb_rate;
float_s32_t g_avg_i2s_rate;
rate_info_t g_i2s_rate_info;
extern rate_info_t g_usb_rate_info;


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

    m_actual_rate_ratio_f = actual_rate_ratio;// - 1e-8; // Optionally, add an extra drift
    m_actual_rate_ratio = uint64_t(m_actual_rate_ratio_f * ((uint64_t)1 << (28+32)));


    g_avg_i2s_rate = float_div((float_s32_t){(int32_t)m_config->nominal_i2s_rate, 0}, (float_s32_t){100000000, 0});
    g_i2s_rate_info.samples = m_config->nominal_i2s_rate;
    g_i2s_rate_info.ticks = 100000000;

    printf("I2S rate = (0x%x, %d), %.10f\n", g_avg_i2s_rate.mant, g_avg_i2s_rate.exp, ((uint32_t)g_avg_i2s_rate.mant)*pow(2, g_avg_i2s_rate.exp));

    SC_THREAD(process); sensitive << trigger;
}

static inline int32_t get_avg_window_size_log2(uint32_t i2s_rate)
{
    if((i2s_rate == 192000) || (i2s_rate == 176400))
    {
        return 12;
    }
    else if((i2s_rate == 96000) || (i2s_rate == 88200))
    {
        return 11;
    }
    else if((i2s_rate == 48000) || (i2s_rate == 44100))
    {
        // Ideally, the windows size should be 2**10 for 48000,44100 so we can average over the same time window worth of samples, but I can't
        // stable monotonically increasing or descreasing averages when averaged over 1024 samples, so continuing with a window size of 2**11 itself.
        return 11;
    }
    else
    {
        assert(0);
    }
    return 0;
}

void ASRC::process()
{
    int32_t input[m_block_size];
    int32_t output[int(2*(m_block_size/m_nominal_rate_ratio_f)) * 2];
    uint32_t buffer_writes_count = 0;
    uint64_t rate_ratio;
    if(m_config->usb_timestamps[0].size() != 0)
    {
        rate_ratio = m_nominal_rate_ratio;
    }
    else
    {
        rate_ratio = m_actual_rate_ratio;
    }

    buffer_calc_state_t long_term_buf_state, short_term_buf_state;


    FILE *fp;
    fp = fopen("asrc_output.bin", "wb");

    int32_t window_len_log2 = get_avg_window_size_log2(m_config->nominal_i2s_rate);

    init_calc_buffer_level_state(&long_term_buf_state, window_len_log2, 4);
    init_calc_buffer_level_state(&short_term_buf_state, 9, 4);

    while(true)
    {
        wait();
        uint32_t num_out_samples = wrapper_asrc_process(&m_config->asrc_input_samples[0], &output[0], rate_ratio);

        fwrite(&output[0], sizeof(int32_t), num_out_samples, fp);

        //unsigned int asrc_delay = 60 + (rand() % 20);
        //wait(asrc_delay, SC_US);
        m_buffer->write(num_out_samples);

        calc_avg_buffer_level(&long_term_buf_state, m_buffer->fill_level(), false);
        calc_avg_buffer_level(&short_term_buf_state, m_buffer->fill_level(), false);

        if(long_term_buf_state.flag_stable_avg)
        {
            printf("%d,%d\n", long_term_buf_state.avg_buffer_level, short_term_buf_state.avg_buffer_level);
        }

        // After 16 writes
        buffer_writes_count += 1;

        if(buffer_writes_count == 16)
        {
            //printf("%d\n",m_buffer->fill_level());
            int64_t error;
            if(m_config->usb_timestamps[0].size() != 0)
            {
                rate_ratio = float_div_u64_fixed_output_q_format(g_avg_i2s_rate, g_avg_usb_rate, 28+32);

                // Try doing (g_i2s_rate_info.samples * g_usb_rate_info.ticks) / (g_usb_rate_info.samples * g_i2s_rate_info.ticks).
                // Doesn't seem to show any benefit so not enabling it and sticking to g_avg_i2s_rate/g_avg_usb_rate instead
                //uint64_t test_rate_ratio = test_mult_div(g_i2s_rate_info, g_usb_rate_info);

                //printf("rate_ratio = %llu, test = %llu\n", rate_ratio, test_rate_ratio);
                //rate_ratio = test_rate_ratio;

                error = calc_usb_buffer_based_correction(m_config->nominal_i2s_rate, &long_term_buf_state, &short_term_buf_state);

                rate_ratio = rate_ratio + error;
            }
            else
            {
                error = calc_usb_buffer_based_correction(m_config->nominal_i2s_rate, &long_term_buf_state, &short_term_buf_state);
                rate_ratio = m_actual_rate_ratio + error;
            }


            //printf("rate ratio = 0x%llx\n", rate_ratio);
            buffer_writes_count = 0;

            /*if(first_done)
            {
                printf("%lld,%d,%d\n",error>>16,m_buffer->fill_level(),g_avg_i2s_send_buffer_level);
            }*/
        }
    }
}
