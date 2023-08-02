// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

/* STD headers */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <xcore/hwtimer.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "stream_buffer.h"

/* App headers */
#include "app_conf.h"
#include "platform/driver_instances.h"
#include "src.h"
#include "asrc_utils.h"
#include "i2s_audio.h"

static void recv_frame_from_i2s(int32_t *i2s_rx_data, size_t frame_count)
{
    size_t rx_count =
    rtos_i2s_rx(i2s_ctx,
                (int32_t*) i2s_rx_data,
                frame_count,
                portMAX_DELAY);
    xassert(rx_count == frame_count);

}

#define NUM_I2S_CHANS (2)
static void i2s_audio_recv_task(void *args)
{
    (void)args;
    printf("SAMPLING_RATE_MULTIPLIER = %d\n", SAMPLING_RATE_MULTIPLIER);
    printf("MIC_ARRAY_SAMPLING_FREQ = %d\n", MIC_ARRAY_SAMPLING_FREQ);

    // Create the 2nd channel ASRC task
    asrc_process_frame_ctx_t asrc_ctx;

    static uint32_t prev_in;
    prev_in = get_reference_time();
    // 1 ASRC instance per channel, so 2 for 2 channels. Each ASRC instance processes one channel
    asrc_state_t     asrc_state[NUM_I2S_CHANS][ASRC_CHANNELS_PER_INSTANCE]; //ASRC state machine state
    int              asrc_stack[NUM_I2S_CHANS][ASRC_CHANNELS_PER_INSTANCE][ASRC_STACK_LENGTH_MULT * I2S_TO_USB_ASRC_BLOCK_LENGTH]; //Buffer between filter stages
    asrc_ctrl_t      asrc_ctrl[NUM_I2S_CHANS][ASRC_CHANNELS_PER_INSTANCE];  //Control structure
    asrc_adfir_coefs_t asrc_adfir_coefs[NUM_I2S_CHANS];

    for(int ch=0; ch<NUM_I2S_CHANS; ch++)
    {
        for(int ui = 0; ui < ASRC_CHANNELS_PER_INSTANCE; ui++)
        {
            //Set state, stack and coefs into ctrl structure
            asrc_ctrl[ch][ui].psState                   = &asrc_state[ch][ui];
            asrc_ctrl[ch][ui].piStack                   = asrc_stack[ch][ui];
            asrc_ctrl[ch][ui].piADCoefs                 = asrc_adfir_coefs[ch].iASRCADFIRCoefs;
        }
    }

    //Initialise ASRC

    // Create init ctx for the ch1 asrc running in another thread
    asrc_init_t asrc_init_ctx;
    asrc_init_ctx.fs_in = appconfI2S_AUDIO_SAMPLE_RATE;
    asrc_init_ctx.fs_out = appconfUSB_AUDIO_SAMPLE_RATE;
    asrc_init_ctx.n_in_samples = I2S_TO_USB_ASRC_BLOCK_LENGTH;
    asrc_init_ctx.asrc_ctrl_ptr = &asrc_ctrl[1][0];
    (void) rtos_osal_queue_create(&asrc_init_ctx.asrc_queue, "asrc_q", 1, sizeof(asrc_process_frame_ctx_t*));
    (void) rtos_osal_queue_create(&asrc_init_ctx.asrc_ret_queue, "asrc_ret_q", 1, sizeof(int));
    // Create 2nd channel ASRC task
    (void) rtos_osal_thread_create(
        (rtos_osal_thread_t *) NULL,
        (char *) "ASRC_1ch",
        (rtos_osal_entry_function_t) asrc_one_channel_task,
        (void *) (&asrc_init_ctx),
        (size_t) RTOS_THREAD_STACK_SIZE(asrc_one_channel_task),
        (unsigned int) appconfAUDIO_PIPELINE_TASK_PRIORITY);


    // Initialise CH0 ASRC context
    fs_code_t in_fs_code = samp_rate_to_code(asrc_init_ctx.fs_in);  //Sample rate code 0..5
    fs_code_t out_fs_code = samp_rate_to_code(asrc_init_ctx.fs_out);
    unsigned nominal_fs_ratio = asrc_init(in_fs_code, out_fs_code, &asrc_ctrl[0][0], ASRC_CHANNELS_PER_INSTANCE, asrc_init_ctx.n_in_samples, ASRC_DITHER_SETTING);
    printf("Input ASRC: nominal_fs_ratio = %d\n", nominal_fs_ratio);
#if 0
    port_t p_bclk_count = XS1_PORT_1N;
    port_enable(p_bclk_count);
    port_set_clock(p_bclk_count, I2S_CLKBLK);

    port_t p_mclk_count = XS1_PORT_1M;
    port_enable(p_mclk_count);
    port_set_clock(p_mclk_count, PDM_CLKBLK_1);

    uint32_t trigger_time, prev_trigger_time;
    prev_trigger_time = port_get_trigger_time(p_bclk_count);
    uint32_t mclk_trigger_time, prev_mclk_trigger_time;
    prev_mclk_trigger_time = port_get_trigger_time(p_mclk_count);
#endif
    int32_t frame_samples[appconfAUDIO_PIPELINE_CHANNELS][I2S_TO_USB_ASRC_BLOCK_LENGTH*2];
    int32_t frame_samples_interleaved[I2S_TO_USB_ASRC_BLOCK_LENGTH*2][appconfAUDIO_PIPELINE_CHANNELS];
    for(;;)
    {
        int32_t tmp[I2S_TO_USB_ASRC_BLOCK_LENGTH][appconfAUDIO_PIPELINE_CHANNELS];
        int32_t tmp_deinterleaved[appconfAUDIO_PIPELINE_CHANNELS][I2S_TO_USB_ASRC_BLOCK_LENGTH];


        recv_frame_from_i2s(&tmp[0][0], I2S_TO_USB_ASRC_BLOCK_LENGTH); // Receive blocks of I2S_TO_USB_ASRC_BLOCK_LENGTH at I2S sampling rate
        for(int ch=0; ch<appconfAUDIO_PIPELINE_CHANNELS; ch++)
        {
            for(int sample=0; sample<I2S_TO_USB_ASRC_BLOCK_LENGTH; sample++)
            {
                tmp_deinterleaved[ch][sample] = tmp[sample][ch];
            }
        }

        // Send to the other channel ASRC task
        asrc_ctx.input_samples = &tmp_deinterleaved[1][0];
        asrc_ctx.output_samples = &frame_samples[1][0];
        asrc_ctx.nominal_fs_ratio = nominal_fs_ratio;
        asrc_process_frame_ctx_t *ptr = &asrc_ctx;

        (void) rtos_osal_queue_send(&asrc_init_ctx.asrc_queue, &ptr, RTOS_OSAL_WAIT_FOREVER);

        // Call asrc on this block of samples. Reuse frame_samples now that its copied into aec_reference_audio_samples
        // Only channel 0 for now
        //uint32_t start = get_reference_time();
        unsigned n_samps_out = asrc_process((int *)&tmp_deinterleaved[0][0], (int *)&frame_samples[0][0], nominal_fs_ratio, &asrc_ctrl[0][0]);
        //uint32_t end = get_reference_time();
        //printuintln(end - start);

        // Wait for 2nd channel ASRC to finish

        unsigned n_samps_out_ch1;
        rtos_osal_queue_receive(&asrc_init_ctx.asrc_ret_queue, &n_samps_out_ch1, RTOS_OSAL_WAIT_FOREVER);

        if(n_samps_out != n_samps_out_ch1)
        {
            printf("Error: I2S to USB ASRC. ch0 and ch1 returned different number of samples: ch0 %u, ch1 %u\n", n_samps_out, n_samps_out_ch1);
            xassert(0);
        }
        uint32_t min_samples = (n_samps_out < n_samps_out_ch1) ? n_samps_out : n_samps_out_ch1;

        for(int i=0; i<min_samples; i++)
        {
            for(int ch=0; ch<appconfAUDIO_PIPELINE_CHANNELS; ch++)
            {
                frame_samples_interleaved[i][ch] = frame_samples[ch][i];
            }
        }

        if (min_samples > 0) {
            rtos_intertile_tx(
                    intertile_ctx,
                    appconfAUDIOPIPELINE_PORT,
                    frame_samples_interleaved,
                    min_samples*appconfAUDIO_PIPELINE_CHANNELS*sizeof(int32_t));
        }
    }
}

void i2s_audio_recv_init()
{
#if ON_TILE(1)
    // Create pipeline input task
    (void) rtos_osal_thread_create(
        (rtos_osal_thread_t *) NULL,
        (char *) "Pipeline_input",
        (rtos_osal_entry_function_t) i2s_audio_recv_task,
        (void *) NULL,
        (size_t) RTOS_THREAD_STACK_SIZE(i2s_audio_recv_task),
        (unsigned int) appconfAUDIO_PIPELINE_TASK_PRIORITY);
#endif
}
