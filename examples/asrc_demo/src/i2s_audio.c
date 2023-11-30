// Copyright 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#define DEBUG_UNIT I2S_AUDIO
#define DEBUG_PRINT_ENABLE_I2S_AUDIO 0

/* STD headers */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "rtos_printf.h"
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
#include "rate_server.h"
#include "tusb_config.h"

static void recv_frame_from_i2s(int32_t *i2s_rx_data, size_t frame_count)
{
    size_t rx_count =
    rtos_i2s_rx(i2s_ctx,
                (int32_t*) i2s_rx_data,
                frame_count,
                portMAX_DELAY);

    xassert(rx_count == frame_count);

}

static void i2s_audio_recv_task(void *args)
{
    (void)args;

    // Create the 2nd channel ASRC task
    asrc_process_frame_ctx_t asrc_ctx;

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
    asrc_init_ctx.fs_in = 0; // I2S rate is detected at runtime
    asrc_init_ctx.fs_out = appconfUSB_AUDIO_SAMPLE_RATE;
    asrc_init_ctx.n_in_samples = I2S_TO_USB_ASRC_BLOCK_LENGTH;
    asrc_init_ctx.asrc_ctrl_ptr = &asrc_ctrl[1][0];
    (void) rtos_osal_queue_create(&asrc_init_ctx.asrc_queue, "asrc_q", 1, sizeof(asrc_process_frame_ctx_t*));
    (void) rtos_osal_queue_create(&asrc_init_ctx.asrc_ret_queue, "asrc_ret_q", 1, sizeof(int));

    rtos_osal_thread_t asrc_ch1_thread;
    // Create 2nd channel ASRC task
    (void) rtos_osal_thread_create(
        (rtos_osal_thread_t *) &asrc_ch1_thread,
        (char *) "ASRC_1ch",
        (rtos_osal_entry_function_t) asrc_one_channel_task,
        (void *) (&asrc_init_ctx),
        (size_t) RTOS_THREAD_STACK_SIZE(asrc_one_channel_task),
        (unsigned int) appconfAUDIO_PIPELINE_TASK_PRIORITY);

    // Keep receiving and discarding from I2S till we get a valid sampling rate
    int32_t input_data[I2S_TO_USB_ASRC_BLOCK_LENGTH][NUM_I2S_CHANS];
    int32_t input_data_deinterleaved[NUM_I2S_CHANS][I2S_TO_USB_ASRC_BLOCK_LENGTH];
    uint32_t i2s_sampling_rate = 0;
    uint32_t new_i2s_sampling_rate = 0;

    // Initialise CH0 ASRC context
    fs_code_t in_fs_code;
    fs_code_t out_fs_code;

    int32_t frame_samples[NUM_I2S_CHANS][I2S_TO_USB_ASRC_BLOCK_LENGTH*2];
    int32_t frame_samples_interleaved[I2S_TO_USB_ASRC_BLOCK_LENGTH*2][NUM_I2S_CHANS];
#if PROFILE_ASRC
    uint32_t max_time = 0;
#endif
    uint64_t nominal_fs_ratio = 0;
    for(;;)
    {
        recv_frame_from_i2s(&input_data[0][0], I2S_TO_USB_ASRC_BLOCK_LENGTH); // Receive blocks of I2S_TO_USB_ASRC_BLOCK_LENGTH at I2S sampling rate

        new_i2s_sampling_rate = rtos_i2s_get_nominal_sampling_rate(i2s_ctx);

        if(new_i2s_sampling_rate == 0) {
            continue;
        }
        else if(new_i2s_sampling_rate != i2s_sampling_rate)
        {
            set_i2s_to_usb_rate_ratio(0); // Since this is updated only at rate monitor trigger interval, set it to 0 so
                                         //we don't end up using the wrong ratio till its updated in the rate monitor
            i2s_sampling_rate = new_i2s_sampling_rate;
            asrc_init_ctx.fs_in = i2s_sampling_rate;
            in_fs_code = samp_rate_to_code(asrc_init_ctx.fs_in);  //Sample rate code 0..5
            out_fs_code = samp_rate_to_code(asrc_init_ctx.fs_out);

            // Reinitialise both channel ASRCs
            nominal_fs_ratio = asrc_init(in_fs_code, out_fs_code, &asrc_ctrl[0][0], ASRC_CHANNELS_PER_INSTANCE, asrc_init_ctx.n_in_samples, ASRC_DITHER_SETTING);
            nominal_fs_ratio = asrc_init(in_fs_code, out_fs_code, &asrc_ctrl[1][0], ASRC_CHANNELS_PER_INSTANCE, asrc_init_ctx.n_in_samples, ASRC_DITHER_SETTING);

            // We're too late to do the asrc_process(), skip this frame
            continue;
        }
        uint64_t current_rate_ratio = nominal_fs_ratio;
        uint64_t rate_ratio = get_i2s_to_usb_rate_ratio();
        if(rate_ratio != 0)
        {
            current_rate_ratio = rate_ratio;
        }

        for(int ch=0; ch<NUM_I2S_CHANS; ch++)
        {
            for(int sample=0; sample<I2S_TO_USB_ASRC_BLOCK_LENGTH; sample++)
            {
                input_data_deinterleaved[ch][sample] = input_data[sample][ch];
            }
        }

        // Send to the other channel ASRC task
        asrc_ctx.input_samples = &input_data_deinterleaved[1][0];
        asrc_ctx.output_samples = &frame_samples[1][0];
        asrc_ctx.fs_ratio = current_rate_ratio;
        asrc_ctx.i2s_sampling_rate = i2s_sampling_rate;
        asrc_process_frame_ctx_t *ptr = &asrc_ctx;

#if PROFILE_ASRC
        uint32_t start = get_reference_time();
#endif
        (void) rtos_osal_queue_send(&asrc_init_ctx.asrc_queue, &ptr, RTOS_OSAL_WAIT_FOREVER);

        // Call asrc on this block of samples. Reuse frame_samples now that its copied into aec_reference_audio_samples
        unsigned n_samps_out = asrc_process((int *)&input_data_deinterleaved[0][0], (int *)&frame_samples[0][0], current_rate_ratio, &asrc_ctrl[0][0]);

        // Wait for 2nd channel ASRC to finish
        unsigned n_samps_out_ch1;
        rtos_osal_queue_receive(&asrc_init_ctx.asrc_ret_queue, &n_samps_out_ch1, RTOS_OSAL_WAIT_FOREVER);

        if(n_samps_out != n_samps_out_ch1)
        {
            rtos_printf("Error: I2S to USB ASRC. ch0 and ch1 returned different number of samples: ch0 %u, ch1 %u\n", n_samps_out, n_samps_out_ch1);
            xassert(0);
        }

        for(int i=0; i<n_samps_out; i++)
        {
            for(int ch=0; ch<NUM_I2S_CHANS; ch++)
            {
                frame_samples_interleaved[i][ch] = frame_samples[ch][i];
            }
        }

#if PROFILE_ASRC
        uint32_t end = get_reference_time();
        if(max_time < (end - start))
        {
            max_time = end - start;
            printchar('i');
            printuintln(max_time);
        }
#endif

        if (n_samps_out > 0) {
            // Send nominal I2S sampling rate
            rtos_intertile_tx(
                intertile_i2s_audio_ctx,
                appconfAUDIOPIPELINE_PORT,
                &i2s_sampling_rate,
                sizeof(uint32_t));

            // Send ASRC output data
            rtos_intertile_tx(
                    intertile_i2s_audio_ctx,
                    appconfAUDIOPIPELINE_PORT,
                    frame_samples_interleaved,
                    n_samps_out*NUM_I2S_CHANS*sizeof(int32_t));
        }
    }
}

static unsigned usb_audio_recv(rtos_intertile_t *intertile_ctx,
                        int32_t **frame_buffers)
{
    static int32_t frame_samples_interleaved[(USB_TO_I2S_ASRC_BLOCK_LENGTH * 4) + 10][CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX]; //+1 should be okay but +10 just in case

    size_t bytes_received;

    bytes_received = rtos_intertile_rx_len(
        intertile_ctx,
        appconfUSB_AUDIO_PORT,
        portMAX_DELAY);

    if (bytes_received > 0)
    {
        xassert(bytes_received <= sizeof(frame_samples_interleaved));

        rtos_intertile_rx_data(
            intertile_ctx,
            frame_samples_interleaved,
            bytes_received);

        *frame_buffers = &frame_samples_interleaved[0][0];
        return bytes_received >> 3; // Return number of 32bit samples per channel. 4bytes per samples and 2 channels
    }
    else
    {
        return 0;
    }
}


// USB audio recv -> ASRC -> |to other tile| -> usb_to_i2s_intertile -> I2S send
static void usb_to_i2s_intertile(void *args) {
    (void) args;
    int32_t *usb_to_i2s_samps;

    init_calc_i2s_buffer_level_state();

    for(;;)
    {
        // Receive USB recv + ASRC data frame from the other tile
        unsigned num_samps = usb_audio_recv(intertile_usb_audio_ctx,
                                            &usb_to_i2s_samps
                                        );

        static uint32_t prev_i2s_sampling_rate = 0;

        if(num_samps)
        {
            int32_t i2s_send_buffer_unread = rtos_i2s_get_send_buffer_unread(i2s_ctx);
            // If we've had a spkr itf close -> open event, we need to ensure the I2S send buffer is at a stable level before we start sending over I2S again.

            // First empty what's in the buffer by sending it over I2S, then stop sending over I2S and wait for the buffer to be atleast half full
            // before resuming sending over I2S again
            if(get_spkr_itf_close_open_event() == true)
            {
                if(i2s_send_buffer_unread > 0)
                {
                    rtos_i2s_set_okay_to_send(i2s_ctx, true);
                    rtos_printf("USB spkr interface opened. i2s_send_buffer_unread = %d\n", i2s_send_buffer_unread);
                    // Wait for i2s send buffer to drain fully before refilling it, so there's no chance we start at a fill level greater than 0
                    continue;
                }
                rtos_printf("USB spkr interface opened. i2s_send_buffer_unread = %d\n", i2s_send_buffer_unread);
                set_spkr_itf_close_open_event(false);
                rtos_i2s_set_okay_to_send(i2s_ctx, false); // We wait for buffer to be half full before resuming send on I2S
            }
            uint32_t i2s_nominal_sampling_rate = rtos_i2s_get_nominal_sampling_rate(i2s_ctx);
            // Similarly, If there's a change in I2S sampling rate, empty the buffer, then stop sending over I2S till buffer is half full and start sending again
            if((i2s_nominal_sampling_rate != 0) && (prev_i2s_sampling_rate != i2s_nominal_sampling_rate))
            {
                if(i2s_send_buffer_unread > 0)
                {
                    rtos_i2s_set_okay_to_send(i2s_ctx, true);
                    rtos_printf("I2S sampling rate change detected. prev = %u, current = %u. i2s_send_buffer_unread = %d\n", prev_i2s_sampling_rate, i2s_nominal_sampling_rate, i2s_send_buffer_unread);
                    // Wait for i2s send buffer to drain fully before refilling it, so there's no chance we start at a fill level greater than 0
                    continue;
                }
                rtos_printf("I2S sampling rate change detected. prev = %u, current = %u. i2s_send_buffer_unread = %d\n", prev_i2s_sampling_rate, i2s_nominal_sampling_rate, i2s_send_buffer_unread);
                prev_i2s_sampling_rate = i2s_nominal_sampling_rate;
                rtos_i2s_set_okay_to_send(i2s_ctx, false);
            }

            rtos_i2s_tx(i2s_ctx,
                (int32_t*) usb_to_i2s_samps,
                num_samps,
                portMAX_DELAY);

            bool okay_to_send = rtos_i2s_get_okay_to_send(i2s_ctx);
            int32_t i2s_buffer_level_from_half = rtos_i2s_get_send_buffer_level_wrt_half(i2s_ctx) / 2; // Per channel

            calc_avg_i2s_send_buffer_level(i2s_buffer_level_from_half, !okay_to_send);

            // If we're not sending and buffer has become half full start sending again so we start at a very stable point
            if((okay_to_send == false) && (i2s_buffer_level_from_half >= 0))
            {
                rtos_i2s_set_okay_to_send(i2s_ctx, true);
                rtos_printf("Start sending over I2S. I2S send buffer fill level = %d\n", i2s_buffer_level_from_half);
            }

            //printintln(i2s_buffer_level_from_half);
        }
    }
}

void i2s_audio_init()
{
    // I2S audio recv + ASRC task
    (void) rtos_osal_thread_create(
        NULL,
         "i2s_audio_recv_asrc",
        i2s_audio_recv_task,
        NULL,
        RTOS_THREAD_STACK_SIZE(i2s_audio_recv_task),
        appconfAUDIO_PIPELINE_TASK_PRIORITY);

    // Rate monitor task
    (void) rtos_osal_thread_create(
        NULL,
        "Rate Server",
        rate_server,
        NULL,
        RTOS_THREAD_STACK_SIZE(rate_server),
        appconfAUDIO_PIPELINE_TASK_PRIORITY);

    // Task for receiving USB+ASRC audio frame from the USB to the I2S tile
    (void) rtos_osal_thread_create(
        NULL,
        "usb_to_i2s_intertile",
        usb_to_i2s_intertile,
        NULL,
        RTOS_THREAD_STACK_SIZE(usb_to_i2s_intertile),
        appconfAUDIO_PIPELINE_TASK_PRIORITY);
}
