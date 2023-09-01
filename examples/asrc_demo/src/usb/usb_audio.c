// Copyright 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Reinhard Panhuber
 * Copyright (c) 2021 XMOS LIMITED
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */
#define DEBUG_UNIT USB_AUDIO
#define DEBUG_PRINT_ENABLE_USB_AUDIO 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <rtos_printf.h>
#include <xcore/hwtimer.h>
#include <src.h>

#include "FreeRTOS.h"
#include "stream_buffer.h"

#include "usb_descriptors.h"
#include "tusb.h"

#include "rtos_intertile.h"

#include "app_conf.h"

#include "asrc_utils.h"
#include "rate_server.h"

// Audio controls
// Current states
bool mute[CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX + 1];       // +1 for master channel 0
uint16_t volume[CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX + 1]; // +1 for master channel 0
uint32_t sampFreq;
uint8_t clkValid;

// Range states
audio_control_range_2_n_t(1) volumeRng[CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX + 1]; // Volume range state
audio_control_range_4_n_t(1) sampleFreqRng;                                     // Sample frequency range state

static volatile bool mic_interface_open = false;
static volatile bool spkr_interface_open = false;

static uint32_t prev_n_bytes_received = 0;
static bool host_streaming_out = false;

StreamBufferHandle_t samples_to_host_stream_buf;
static StreamBufferHandle_t samples_from_host_stream_buf;
static StreamBufferHandle_t rx_buffer;
static TaskHandle_t usb_audio_out_asrc_handle;

static uint32_t g_usb_to_i2s_rate_ratio = 0;
static uint32_t samples_to_host_stream_buf_size_bytes = 0;
static bool g_i2s_sr_change_detected = false;

extern float_s32_t g_usb_data_rate[2];


#define USB_FRAMES_PER_ASRC_INPUT_FRAME (USB_TO_I2S_ASRC_BLOCK_LENGTH / (appconfUSB_AUDIO_SAMPLE_RATE / 1000))

static uint32_t g_i2s_nominal_sampling_rate = 0;
void update_i2s_nominal_sampling_rate(uint32_t i2s_rate)
{
    g_i2s_nominal_sampling_rate = i2s_rate;
}

uint32_t get_i2s_nominal_sampling_rate()
{
    return g_i2s_nominal_sampling_rate;
}


//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
    rtos_printf("USB mounted\n");
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
    rtos_printf("USB unmounted\n");
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
    (void)remote_wakeup_en;
    xassert(false);
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
}

//--------------------------------------------------------------------+
// AUDIO Task
//--------------------------------------------------------------------+

#if CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX == 2
typedef int16_t samp_t;
#elif CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX == 4
typedef int32_t samp_t;
#else
#error CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX must be either 2 or 4
#endif


void usb_audio_send(int32_t *frame_buffer_ptr, // buffer containing interleaved samples [samps][ch] format
                    size_t frame_count,
                    size_t num_chans)
{
    #define RATE_MONITOR_TRIGGER_INTERVAL (16)
    static int32_t prev_i2s_sampling_rate = 0;
    static uint32_t num_samples_to_host_buf_writes = 0;
    static uint32_t num_dummy_writes = 0;

    samp_t usb_audio_in_frame[I2S_TO_USB_ASRC_BLOCK_LENGTH * 2][CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX];
#if CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX == 2
    const int src_32_shift = 16;
#elif CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX == 4
    const int src_32_shift = 0;
#endif

    memset(usb_audio_in_frame, 0, sizeof(usb_audio_in_frame));
    for (int i = 0; i < frame_count; i++)
    {
        for (int ch = 0; ch < num_chans; ch++)
        {
            usb_audio_in_frame[i][ch] = frame_buffer_ptr[i * 2 + ch] >> src_32_shift;
        }
    }
    size_t usb_audio_in_size_bytes = frame_count * num_chans * sizeof(samp_t);

    usb_to_i2s_rate_info_t usb_rate_info;
    usb_rate_info.mic_itf_open = mic_interface_open;
    usb_rate_info.spkr_itf_open = spkr_interface_open;
    usb_rate_info.usb_data_rate[TUSB_DIR_IN] = g_usb_data_rate[TUSB_DIR_IN];
    usb_rate_info.usb_data_rate[TUSB_DIR_OUT] = g_usb_data_rate[TUSB_DIR_OUT];
    usb_rate_info.samples_to_host_buf_fill_level = 0;

    bool intertile_send = false;
    if (usb_rate_info.mic_itf_open)
    {
        uint32_t current_i2s_rate = get_i2s_nominal_sampling_rate();

        if((prev_i2s_sampling_rate != current_i2s_rate) && (current_i2s_rate != 0))
        {
            rtos_printf("I2S SR change detected in usb_audio_send(). prev SR %d, new SR %d\n", prev_i2s_sampling_rate, current_i2s_rate);
            // Set this flag and wait for it to be cleared from tud_audio_tx_done_pre_load_cb(), which it will, after resetting the samples_to_host_stream_buf. We wait
            // for g_i2s_sr_change_detected to be False before starting to write in the samples_to_host_stream_buf.
            g_i2s_sr_change_detected = true;
            num_samples_to_host_buf_writes = 0;
            num_dummy_writes = 0;
        }
        prev_i2s_sampling_rate = current_i2s_rate;

        if(g_i2s_sr_change_detected == false)
        {
            if (xStreamBufferSpacesAvailable(samples_to_host_stream_buf) >= usb_audio_in_size_bytes)
            {
                xStreamBufferSend(samples_to_host_stream_buf, usb_audio_in_frame, usb_audio_in_size_bytes, 0);

                num_samples_to_host_buf_writes += 1;
                if(num_samples_to_host_buf_writes % RATE_MONITOR_TRIGGER_INTERVAL == 0)
                {
                    int usb_buffer_level_from_half = (signed)xStreamBufferBytesAvailable(samples_to_host_stream_buf) - (samples_to_host_stream_buf_size_bytes / 2);    //Level w.r.t. half full
                    usb_rate_info.samples_to_host_buf_fill_level = usb_buffer_level_from_half;
                    intertile_send = true; // Trigger rate monitoring on the other tile
                }
            }
            else
            {
                rtos_printf("lost VFE output samples\n");
            }
        }
    }
    else // If mic_interface is not open we send this information anyway since the rate calculation for the spkr interface (i2s in usb out) might need to be done
    {
        num_dummy_writes += 1;
        if(num_dummy_writes % RATE_MONITOR_TRIGGER_INTERVAL == 0)
        {
            intertile_send = true; // Trigger rate monitoring on the other tile
        }
    }
    if(intertile_send == true)
    {
        i2s_to_usb_rate_info_t i2s_rate_info;
        size_t bytes_received;

        rtos_intertile_tx(
        intertile_ctx,
            appconfUSB_RATE_NOTIFY_PORT,
            &usb_rate_info,
            sizeof(usb_rate_info));


        bytes_received = rtos_intertile_rx_len(
            intertile_ctx,
            appconfUSB_RATE_NOTIFY_PORT,
            portMAX_DELAY);
        xassert(bytes_received == sizeof(i2s_rate_info));

        rtos_intertile_rx_data(
            intertile_ctx,
            &i2s_rate_info,
            bytes_received);

        // Update ratio only when both rates are valid
        g_usb_to_i2s_rate_ratio = i2s_rate_info.usb_to_i2s_rate_ratio;

    }
}

//
/**
 * @brief Task that performs ASRC on the USB OUT data and send the ASRC output frame to the I2S tile
 *
 * Runs on the USB tile.
 * @param arg Handle to the intertile ctx over which ASRC output is sent to the I2S tile
 */
void usb_audio_out_asrc(void *arg)
{

#if CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX == 2
    const int src_32_shift = 16;
#elif CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX == 4
    const int src_32_shift = 0;
#endif

    rtos_intertile_t *intertile_ctx = (rtos_intertile_t *)arg;

    // Initialise channel 0 ASRC instance
    asrc_state_t asrc_state[CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX][ASRC_CHANNELS_PER_INSTANCE];                                               // ASRC state machine state
    int asrc_stack[CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX][ASRC_CHANNELS_PER_INSTANCE][ASRC_STACK_LENGTH_MULT * USB_TO_I2S_ASRC_BLOCK_LENGTH]; // Buffer between filter stages
    asrc_ctrl_t asrc_ctrl[CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX][ASRC_CHANNELS_PER_INSTANCE];                                                 // Control structure
    asrc_adfir_coefs_t asrc_adfir_coefs[CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX];

    for (int ch = 0; ch < CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX; ch++)
    {
        for (int ui = 0; ui < ASRC_CHANNELS_PER_INSTANCE; ui++)
        {
            // Set state, stack and coefs into ctrl structure
            asrc_ctrl[ch][ui].psState = &asrc_state[ch][ui];
            asrc_ctrl[ch][ui].piStack = asrc_stack[ch][ui];
            asrc_ctrl[ch][ui].piADCoefs = asrc_adfir_coefs[ch].iASRCADFIRCoefs;
        }
    }

    // Initialise ASRC
    //  Create init ctx for the ch1 asrc running in another thread
    asrc_init_t asrc_init_ctx;
    asrc_init_ctx.fs_in = appconfUSB_AUDIO_SAMPLE_RATE;
    asrc_init_ctx.fs_out = 0; // Will be notified at runtime
    asrc_init_ctx.n_in_samples = USB_TO_I2S_ASRC_BLOCK_LENGTH;
    asrc_init_ctx.asrc_ctrl_ptr = &asrc_ctrl[1][0];
    (void)rtos_osal_queue_create(&asrc_init_ctx.asrc_queue, "asrc_q", 1, sizeof(asrc_process_frame_ctx_t *));
    (void)rtos_osal_queue_create(&asrc_init_ctx.asrc_ret_queue, "asrc_ret_q", 1, sizeof(int));

    rtos_osal_thread_t asrc_ch1_thread;

    // Create 2nd channel ASRC task
    (void)rtos_osal_thread_create(
        (rtos_osal_thread_t *)&asrc_ch1_thread,
        (char *)"ASRC_1ch",
        (rtos_osal_entry_function_t)asrc_one_channel_task,
        (void *)(&asrc_init_ctx),
        (size_t)RTOS_THREAD_STACK_SIZE(asrc_one_channel_task),
        (unsigned int)appconfAUDIO_PIPELINE_TASK_PRIORITY);

    fs_code_t in_fs_code;
    fs_code_t out_fs_code;
    unsigned nominal_fs_ratio;
#if PROFILE_ASRC
    uint32_t max_time = 0;
#endif

    int32_t frame_samples[CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX][USB_TO_I2S_ASRC_BLOCK_LENGTH * 4 + USB_TO_I2S_ASRC_BLOCK_LENGTH];             // TODO calculate size properly
    int32_t frame_samples_interleaved[USB_TO_I2S_ASRC_BLOCK_LENGTH * 4 + USB_TO_I2S_ASRC_BLOCK_LENGTH][CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX]; // TODO calculate size properly

    asrc_process_frame_ctx_t asrc_ctx;
    for (;;)
    {
        samp_t usb_audio_out_frame[USB_TO_I2S_ASRC_BLOCK_LENGTH][CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX];
        int32_t usb_audio_out_frame_deinterleaved[CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX][USB_TO_I2S_ASRC_BLOCK_LENGTH];
        size_t bytes_received = 0;

        /*
         * Only wake up when the stream buffer contains a whole audio
         * pipeline frame.
         */
        (void)ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
        bytes_received = xStreamBufferReceive(samples_from_host_stream_buf, usb_audio_out_frame, sizeof(usb_audio_out_frame), 0);
        uint32_t current_i2s_rate = get_i2s_nominal_sampling_rate();

        if(current_i2s_rate == 0)
        {
            continue;
        }
        if (asrc_init_ctx.fs_out != current_i2s_rate)
        {
            // Time to initialise asrc
            g_usb_to_i2s_rate_ratio = 0;
            asrc_init_ctx.fs_out = current_i2s_rate;
            in_fs_code = samp_rate_to_code(asrc_init_ctx.fs_in); // Sample rate code 0..5
            out_fs_code = samp_rate_to_code(asrc_init_ctx.fs_out);
            rtos_printf("USB tile initialising ASRC for fs_in %lu, fs_out %lu\n", asrc_init_ctx.fs_in, asrc_init_ctx.fs_out);

            // Initialise both channel ASRCs
            nominal_fs_ratio = asrc_init(in_fs_code, out_fs_code, &asrc_ctrl[0][0], ASRC_CHANNELS_PER_INSTANCE, asrc_init_ctx.n_in_samples, ASRC_DITHER_SETTING);
            nominal_fs_ratio = asrc_init(in_fs_code, out_fs_code, &asrc_ctrl[1][0], ASRC_CHANNELS_PER_INSTANCE, asrc_init_ctx.n_in_samples, ASRC_DITHER_SETTING);
            // Skip this frame since we're too late anayway from 2 asrc_init() calls, each taking 12500 cycles
            continue;
        }

        uint32_t current_rate_ratio = nominal_fs_ratio;
        if(g_usb_to_i2s_rate_ratio != 0)
        {
            current_rate_ratio = g_usb_to_i2s_rate_ratio;
        }

        for (int ch = 0; ch < CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX; ch++)
        {
            for (int i = 0; i < USB_TO_I2S_ASRC_BLOCK_LENGTH; i++)
            {
                usb_audio_out_frame_deinterleaved[ch][i] = usb_audio_out_frame[i][ch] << src_32_shift;
            }
        }

        // Send to the other channel ASRC task
        asrc_ctx.input_samples = &usb_audio_out_frame_deinterleaved[1][0];
        asrc_ctx.output_samples = &frame_samples[1][0];
        asrc_ctx.nominal_fs_ratio = current_rate_ratio;
        asrc_process_frame_ctx_t *ptr = &asrc_ctx;
#if PROFILE_ASRC
        uint32_t start = get_reference_time();
#endif

        (void)rtos_osal_queue_send(&asrc_init_ctx.asrc_queue, &ptr, RTOS_OSAL_WAIT_FOREVER);

        // Call asrc on this block of samples. Reuse frame_samples now that its copied into aec_reference_audio_samples

        unsigned n_samps_out = asrc_process((int *)&usb_audio_out_frame_deinterleaved[0][0], (int *)&frame_samples[0][0], current_rate_ratio, &asrc_ctrl[0][0]);


        unsigned n_samps_out_ch1;
        rtos_osal_queue_receive(&asrc_init_ctx.asrc_ret_queue, &n_samps_out_ch1, RTOS_OSAL_WAIT_FOREVER);

        if (n_samps_out != n_samps_out_ch1)
        {
            rtos_printf("Error: USB to I2S ASRC. ch0 and ch1 returned different number of samples: ch0 %u, ch1 %u\n", n_samps_out, n_samps_out_ch1);
            xassert(0);
        }

        for (int ch = 0; ch < CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX; ch++)
        {
            for (int i = 0; i < n_samps_out; i++)
            {
                frame_samples_interleaved[i][ch] = frame_samples[ch][i];
            }
        }
#if PROFILE_ASRC
        uint32_t end = get_reference_time();
        if(max_time < (end - start))
        {
            max_time = end - start;
            printchar('u');
            printuintln(max_time);
        }
#endif

        /*
         * This shouldn't normally be zero, but it could be possible that
         * the stream buffer is reset after this task has been notified.
         */
        if (n_samps_out > 0)
        {
            rtos_intertile_tx(
                intertile_ctx,
                appconfUSB_AUDIO_PORT,
                frame_samples_interleaved,
                n_samps_out * CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX * sizeof(int32_t));
        }
    }
}

//--------------------------------------------------------------------+
// Application Callback API Implementations
//--------------------------------------------------------------------+

// Invoked when audio class specific set request received for an EP
bool tud_audio_set_req_ep_cb(uint8_t rhport,
                             tusb_control_request_t const *p_request,
                             uint8_t *pBuff)
{
    (void)rhport;
    (void)pBuff;

    // We do not support any set range requests here, only current value requests
    TU_VERIFY(p_request->bRequest == AUDIO_CS_REQ_CUR);

    // Page 91 in UAC2 specification
    uint8_t channelNum = TU_U16_LOW(p_request->wValue);
    uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
    uint8_t ep = TU_U16_LOW(p_request->wIndex);

    (void)channelNum;
    (void)ctrlSel;
    (void)ep;

    return false; // Yet not implemented
}

// Invoked when audio class specific set request received for an interface
bool tud_audio_set_req_itf_cb(uint8_t rhport,
                              tusb_control_request_t const *p_request,
                              uint8_t *pBuff)
{
    (void)rhport;
    (void)pBuff;

    // We do not support any set range requests here, only current value requests
    TU_VERIFY(p_request->bRequest == AUDIO_CS_REQ_CUR);

    // Page 91 in UAC2 specification
    uint8_t channelNum = TU_U16_LOW(p_request->wValue);
    uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
    uint8_t itf = TU_U16_LOW(p_request->wIndex);

    (void)channelNum;
    (void)ctrlSel;
    (void)itf;

    return false; // Yet not implemented
}

// Invoked when audio class specific set request received for an entity
bool tud_audio_set_req_entity_cb(uint8_t rhport,
                                 tusb_control_request_t const *p_request,
                                 uint8_t *pBuff)
{
    (void)rhport;

    // Page 91 in UAC2 specification
    uint8_t channelNum = TU_U16_LOW(p_request->wValue);
    uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
    uint8_t itf = TU_U16_LOW(p_request->wIndex);
    uint8_t entityID = TU_U16_HIGH(p_request->wIndex);

    (void)itf;

    // We do not support any set range requests here, only current value requests
    TU_VERIFY(p_request->bRequest == AUDIO_CS_REQ_CUR);

    // If request is for our feature unit
    if (entityID == UAC2_ENTITY_MIC_FEATURE_UNIT)
    {
        switch (ctrlSel)
        {
        case AUDIO_FU_CTRL_MUTE:
            // Request uses format layout 1
            TU_VERIFY(p_request->wLength == sizeof(audio_control_cur_1_t));

            mute[channelNum] = ((audio_control_cur_1_t *)pBuff)->bCur;

            TU_LOG2("    Set Mute: %d of channel: %u\r\n", mute[channelNum], channelNum);

            return true;

        case AUDIO_FU_CTRL_VOLUME:
            // Request uses format layout 2
            TU_VERIFY(p_request->wLength == sizeof(audio_control_cur_2_t));

            volume[channelNum] = ((audio_control_cur_2_t *)pBuff)->bCur;

            TU_LOG2("    Set Volume: %d dB of channel: %u\r\n", volume[channelNum], channelNum);

            return true;

            // Unknown/Unsupported control
        default:
            TU_BREAKPOINT();
            return false;
        }
    }
    return false; // Yet not implemented
}

// Invoked when audio class specific get request received for an EP
bool tud_audio_get_req_ep_cb(uint8_t rhport,
                             tusb_control_request_t const *p_request)
{
    (void)rhport;

    // Page 91 in UAC2 specification
    uint8_t channelNum = TU_U16_LOW(p_request->wValue);
    uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
    uint8_t ep = TU_U16_LOW(p_request->wIndex);

    (void)channelNum;
    (void)ctrlSel;
    (void)ep;

    //	return tud_control_xfer(rhport, p_request, &tmp, 1);

    return false; // Yet not implemented
}

// Invoked when audio class specific get request received for an interface
bool tud_audio_get_req_itf_cb(uint8_t rhport,
                              tusb_control_request_t const *p_request)
{
    (void)rhport;

    // Page 91 in UAC2 specification
    uint8_t channelNum = TU_U16_LOW(p_request->wValue);
    uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
    uint8_t itf = TU_U16_LOW(p_request->wIndex);

    (void)channelNum;
    (void)ctrlSel;
    (void)itf;

    return false; // Yet not implemented
}

// Invoked when audio class specific get request received for an entity
bool tud_audio_get_req_entity_cb(uint8_t rhport,
                                 tusb_control_request_t const *p_request)
{
    (void)rhport;

    // Page 91 in UAC2 specification
    uint8_t channelNum = TU_U16_LOW(p_request->wValue);
    uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
    // uint8_t itf = TU_U16_LOW(p_request->wIndex); 			// Since we have only one audio function implemented, we do not need the itf value
    uint8_t entityID = TU_U16_HIGH(p_request->wIndex);

    // Input terminal (Microphone input)
    if (entityID == UAC2_ENTITY_MIC_INPUT_TERMINAL)
    {
        switch (ctrlSel)
        {
        case AUDIO_TE_CTRL_CONNECTOR:;
            // The terminal connector control only has a get request with only the CUR attribute.

            audio_desc_channel_cluster_t ret;

            // Those are dummy values for now
            ret.bNrChannels = CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX;
            ret.bmChannelConfig = 0;
            ret.iChannelNames = 0;

            TU_LOG2("    Get terminal connector\r\n");
            rtos_printf("Get terminal connector\r\n");

            return tud_audio_buffer_and_schedule_control_xfer(rhport, p_request, (void *)&ret, sizeof(ret));

            // Unknown/Unsupported control selector
        default:
            TU_BREAKPOINT();
            return false;
        }
    }

    // Feature unit
    if (entityID == UAC2_ENTITY_MIC_FEATURE_UNIT)
    {
        switch (ctrlSel)
        {
        case AUDIO_FU_CTRL_MUTE:
            // Audio control mute cur parameter block consists of only one byte - we thus can send it right away
            // There does not exist a range parameter block for mute
            TU_LOG2("    Get Mute of channel: %u\r\n", channelNum);
            return tud_control_xfer(rhport, p_request, &mute[channelNum], 1);

        case AUDIO_FU_CTRL_VOLUME:

            switch (p_request->bRequest)
            {
            case AUDIO_CS_REQ_CUR:
                TU_LOG2("    Get Volume of channel: %u\r\n", channelNum);
                return tud_control_xfer(rhport, p_request, &volume[channelNum], sizeof(volume[channelNum]));
            case AUDIO_CS_REQ_RANGE:
                TU_LOG2("    Get Volume range of channel: %u\r\n", channelNum);

                // Copy values - only for testing - better is version below
                audio_control_range_2_n_t(1) ret;

                ret.wNumSubRanges = 1;
                ret.subrange[0].bMin = -90; // -90 dB
                ret.subrange[0].bMax = 90;  // +90 dB
                ret.subrange[0].bRes = 1;   // 1 dB steps

                return tud_audio_buffer_and_schedule_control_xfer(rhport, p_request, (void *)&ret, sizeof(ret));

                // Unknown/Unsupported control
            default:
                TU_BREAKPOINT();
                return false;
            }

            // Unknown/Unsupported control
        default:
            TU_BREAKPOINT();
            return false;
        }
    }

    // Clock Source unit
    if (entityID == UAC2_ENTITY_CLOCK)
    {
        switch (ctrlSel)
        {
        case AUDIO_CS_CTRL_SAM_FREQ:

            // channelNum is always zero in this case

            switch (p_request->bRequest)
            {
            case AUDIO_CS_REQ_CUR:
                TU_LOG2("    Get Sample Freq.\r\n");
                return tud_control_xfer(rhport, p_request, &sampFreq, sizeof(sampFreq));
            case AUDIO_CS_REQ_RANGE:
                TU_LOG2("    Get Sample Freq. range\r\n");
                //((tusb_control_request_t *)p_request)->wLength = 14;
                return tud_control_xfer(rhport, p_request, &sampleFreqRng, sizeof(sampleFreqRng));

                // Unknown/Unsupported control
            default:
                TU_BREAKPOINT();
                return false;
            }

        case AUDIO_CS_CTRL_CLK_VALID:
            // Only cur attribute exists for this request
            TU_LOG2("    Get Sample Freq. valid\r\n");
            return tud_control_xfer(rhport, p_request, &clkValid, sizeof(clkValid));

            // Unknown/Unsupported control
        default:
            TU_BREAKPOINT();
            return false;
        }
    }

    TU_LOG2("  Unsupported entity: %d\r\n", entityID);
    return false; // Yet not implemented
}

bool tud_audio_rx_done_pre_read_cb(uint8_t rhport,
                                   uint16_t n_bytes_received,
                                   uint8_t func_id,
                                   uint8_t ep_out,
                                   uint8_t cur_alt_setting)
{
    (void)rhport;
    (void)n_bytes_received;
    (void)func_id;
    (void)ep_out;
    (void)cur_alt_setting;

    return true;
}

bool tud_audio_rx_done_post_read_cb(uint8_t rhport,
                                    uint16_t n_bytes_received,
                                    uint8_t func_id,
                                    uint8_t ep_out,
                                    uint8_t cur_alt_setting)
{
    (void)rhport;

    uint8_t rx_data[CFG_TUD_AUDIO_FUNC_1_EP_OUT_SW_BUF_SZ];
    samp_t usb_audio_frames[AUDIO_FRAMES_PER_USB_FRAME][CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX];
    const size_t stream_buffer_send_byte_count = sizeof(usb_audio_frames);

    host_streaming_out = true;
    prev_n_bytes_received = n_bytes_received;

    if (!spkr_interface_open)
    {
        spkr_interface_open = true;
    }

    /*
     * rx_data is a holding space to recieve the latest USB transaction.
     * If it fits, we then push into rx_buffer. This could be a nominal-size transaction, or it could not be.
     * We then only push nominal size transactions into the stream buffer.
     * Hopefully this doesn't cause timing issues in the pipeline.
     */

    if (sizeof(rx_data) >= n_bytes_received)
    {
        tud_audio_read(rx_data, n_bytes_received);
    }
    else
    {
        /*
         * I don't believe we ever get here because I think the EP FIFO will complain before this,
         * but better safe than sorry
         */
        rtos_printf("Rx'd too much USB data in one transaction, cannot read\n");
        return false;
    }

    if (xStreamBufferSpacesAvailable(rx_buffer) >= n_bytes_received)
    {
        xStreamBufferSend(rx_buffer, rx_data, n_bytes_received, 0);
    }
    else
    {
        rtos_printf("Rx'd too much total USB data, cannot buffer\n");
        return false;
    }

    if (xStreamBufferBytesAvailable(rx_buffer) >= sizeof(usb_audio_frames))
    {
        size_t num_rx_total = 0;
        while (num_rx_total < sizeof(usb_audio_frames))
        {
            size_t num_rx = xStreamBufferReceive(rx_buffer, &usb_audio_frames[num_rx_total], sizeof(usb_audio_frames) - num_rx_total, 0);
            num_rx_total += num_rx;
        }
    }
    else
    {
        rtos_printf("Not enough data to send to stream buffer, cycling again\n");
        return true;
    }

    if (xStreamBufferSpacesAvailable(samples_from_host_stream_buf) >= stream_buffer_send_byte_count)
    {
        xStreamBufferSend(samples_from_host_stream_buf, usb_audio_frames, stream_buffer_send_byte_count, 0);

        /*
         * Wake up the task waiting on this buffer whenever there is one more
         * USB frame worth of audio data than the amount of data required to
         * be input into the pipeline.
         *
         * This way the task will not wake up each time this task puts another
         * milliseconds of audio into the stream buffer, but rather once every
         * pipeline frame time.
         */
        const size_t buffer_notify_level = stream_buffer_send_byte_count * (1 + USB_FRAMES_PER_ASRC_INPUT_FRAME);

        /*
         * TODO: If the above is modified such that not exactly AUDIO_FRAMES_PER_USB_FRAME / RATE_MULTIPLIER
         * frames are written to the stream buffer at a time, then this will need to change to >=.
         */

        if (xStreamBufferBytesAvailable(samples_from_host_stream_buf) == buffer_notify_level)
        {
            xTaskNotifyGive(usb_audio_out_asrc_handle);
        }
    }
    else
    {
        rtos_printf("lost USB output samples. Space available %d, send_byte_count %d\n", xStreamBufferSpacesAvailable(samples_from_host_stream_buf), stream_buffer_send_byte_count);
    }

    return true;
}

bool tud_audio_tx_done_pre_load_cb(uint8_t rhport,
                                   uint8_t itf,
                                   uint8_t ep_in,
                                   uint8_t cur_alt_setting)
{
    (void)rhport;
    (void)itf;
    (void)ep_in;
    (void)cur_alt_setting;

    static int ready = 0;
    size_t bytes_available;
    size_t tx_size_bytes;
    size_t tx_size_frames;
    static bool first_frame_after_mic_interface_open = false;

    /*
     * This buffer needs to be large enough to hold any size of transaction,
     * but if it's any bigger than twice nominal then we have bigger issues
     */
    samp_t stream_buffer_audio_frames[2 * AUDIO_FRAMES_PER_USB_FRAME][CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX];

    /* This buffer has to be large enough to contain any size transaction */
    samp_t usb_audio_frames[2 * AUDIO_FRAMES_PER_USB_FRAME][CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX];

    /*
     * Copying XUA_lite logic basically verbatim - if the host is streaming out,
     * then we send back the number of samples per channel that we last received.
     * If it's not, then we send the nominal number of samples per channel.
     * This assumes (as with XUA_lite) that the host sends the same number of samples for each channel.
     * This also assumes that TX and RX rates are the same, which is an assumption made elsewhere.
     * This finally assumes that at nominal rate,
     *     AUDIO_FRAMES_PER_USB_FRAME / RATE_MULTIPLIER == prev_n_bytes_received / CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX
     */
    if (host_streaming_out && (0 != prev_n_bytes_received))
    {
        tx_size_bytes = (prev_n_bytes_received / CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX) * CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX;
    }
    else
    {
        tx_size_bytes = sizeof(samp_t) * (AUDIO_FRAMES_PER_USB_FRAME)*CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX;
    }
    tx_size_frames = tx_size_bytes / (sizeof(samp_t) * CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX);

    if(first_frame_after_mic_interface_open == true)
    {
        first_frame_after_mic_interface_open = false;
        mic_interface_open = true; // Consider mic interface open for the other end to start filling samples_to_host_stream_buf only when receiving the 2nd frame after itf alt setting is changed.
    }
    else if (!mic_interface_open)
    {
        ready = 0;
        first_frame_after_mic_interface_open = true;
    }

    if(g_i2s_sr_change_detected == true)
    {
        // Change in I2S sampling rate. Reset the buffer and start from fill level = 0 again
        xStreamBufferReset(samples_to_host_stream_buf);
        ready = 0;
        g_i2s_sr_change_detected = false;
        rtos_printf("Resetting samples_to_host_stream_buf due to I2S SR change\n");
    }


    /*
     * If the buffer becomes full, reset it in an attempt to
     * maintain a good fill level again.
     */

    if (xStreamBufferIsFull(samples_to_host_stream_buf))
    {
        xStreamBufferReset(samples_to_host_stream_buf);
        ready = 0;
        rtos_printf("Oops buffer is full\n");
        return true;
    }

    bytes_available = xStreamBufferBytesAvailable(samples_to_host_stream_buf);

    if(bytes_available >= samples_to_host_stream_buf_size_bytes/2) // Buffer fill level 0
    {
        /* wait until we have 2 full audio pipeline output frames in the buffer */
        if(!ready)
        {
            rtos_printf("READY. Fill level = %d\n", xStreamBufferBytesAvailable(samples_to_host_stream_buf) - samples_to_host_stream_buf_size_bytes/2);
        }
        ready = 1;
    }

    if (!ready)
    {
        rtos_printf("TX BUFFER NOT READY, tx_size_bytes = %d\n", tx_size_bytes);
        // we need to send something despite not being fully ready
        //  so, send all zeros
        memset(usb_audio_frames, 0, tx_size_bytes);
        tud_audio_write(usb_audio_frames, tx_size_bytes);
        return true;
    }

    size_t tx_size_bytes_rate_adjusted = tx_size_bytes;

    /* We must always output samples equal to what we recv in adaptive
     * In the event we underflow send 0's. */
    size_t ready_data_bytes = 0;
    if (bytes_available >= tx_size_bytes_rate_adjusted)
    {
        ready_data_bytes = tx_size_bytes_rate_adjusted;
    }
    else
    {
        ready_data_bytes = bytes_available;
        memset(stream_buffer_audio_frames, 0, tx_size_bytes);

        rtos_printf("Oops tx buffer underflowed!\n");
    }

    size_t num_rx_total = 0;
    while (num_rx_total < ready_data_bytes)
    {
        size_t num_rx = xStreamBufferReceive(samples_to_host_stream_buf, &stream_buffer_audio_frames[num_rx_total], ready_data_bytes - num_rx_total, 0);
        num_rx_total += num_rx;
    }

    tud_audio_write(stream_buffer_audio_frames, tx_size_bytes);

    return true;
}

bool tud_audio_tx_done_post_load_cb(uint8_t rhport,
                                    uint16_t n_bytes_copied,
                                    uint8_t itf,
                                    uint8_t ep_in,
                                    uint8_t cur_alt_setting)
{
    (void)rhport;
    (void)n_bytes_copied;
    (void)itf;
    (void)ep_in;
    (void)cur_alt_setting;

    return true;
}

bool tud_audio_set_itf_cb(uint8_t rhport,
                          tusb_control_request_t const *p_request)
{
    (void)rhport;
    uint8_t const itf = tu_u16_low(tu_le16toh(p_request->wIndex));
    uint8_t const alt = tu_u16_low(tu_le16toh(p_request->wValue));

#if AUDIO_OUTPUT_ENABLED
    if (itf == ITF_NUM_AUDIO_STREAMING_SPK)
    {
        /* In case the interface is reset without
         * closing it first */
        spkr_interface_open = false;
        xStreamBufferReset(samples_from_host_stream_buf);
        xStreamBufferReset(rx_buffer);
    }
#endif
#if AUDIO_INPUT_ENABLED
    if (itf == ITF_NUM_AUDIO_STREAMING_MIC)
    {
        /* In case the interface is reset without
         * closing it first */
        mic_interface_open = false;
        xStreamBufferReset(samples_to_host_stream_buf);
        rtos_printf("audio_set_itf: samples_to_host_stream_buf buffer spaces %d\n", xStreamBufferSpacesAvailable(samples_to_host_stream_buf));
    }
#endif

    rtos_printf("Set audio interface %d alt %d\n", itf, alt);

    return true;
}

bool tud_audio_set_itf_close_EP_cb(uint8_t rhport,
                                   tusb_control_request_t const *p_request)
{
    (void)rhport;
    uint8_t const itf = tu_u16_low(tu_le16toh(p_request->wIndex));
    uint8_t const alt = tu_u16_low(tu_le16toh(p_request->wValue));

#if AUDIO_OUTPUT_ENABLED
    if (itf == ITF_NUM_AUDIO_STREAMING_SPK)
    {
        spkr_interface_open = false;
    }
#endif
#if AUDIO_INPUT_ENABLED
    if (itf == ITF_NUM_AUDIO_STREAMING_MIC)
    {
        mic_interface_open = false;
        rtos_printf("audio_set_itf_close: samples_to_host_stream_buf buffer spaces %d\n", xStreamBufferSpacesAvailable(samples_to_host_stream_buf));
    }
#endif

    rtos_printf("Close audio interface %d alt %d\n", itf, alt);

    return true;
}


// I2S recv -> ASRC -> |to other tile| -> i2s_to_usb_intertile -> usb_audio_send
// This task receives I2S nominal sampling rate and the post ASRC data from the I2S tile into the USB tile
static void i2s_to_usb_intertile(void *args)
{
    // When sending from I2S to USB side, we're always downsampling, except for the 44.1 -> 48 case, so the post ASRC buffer length will always be
    // less than I2S_TO_USB_ASRC_BLOCK_LENGTH except for the 44.1 -> 48 case, so we let this decide the buffer size.
    (void) args;
    #define BUFFER_SIZE (((I2S_TO_USB_ASRC_BLOCK_LENGTH * 48000)/44100) + 10) // +1 should be enough but just in case
    int32_t i2s_to_usb_samps_interleaved[BUFFER_SIZE][NUM_I2S_CHANS];
    uint32_t i2s_nominal_sampling_rate;

    for(;;)
    {
        size_t bytes_received;
        // Get I2S nominal sampling rate from I2S tile to USB tile
        bytes_received = rtos_intertile_rx_len(
                intertile_i2s_audio_ctx,
                appconfAUDIOPIPELINE_PORT,
                portMAX_DELAY);
        xassert(bytes_received <= sizeof(i2s_nominal_sampling_rate));

        rtos_intertile_rx_data(
                    intertile_i2s_audio_ctx,
                    &i2s_nominal_sampling_rate,
                    bytes_received);

        update_i2s_nominal_sampling_rate(i2s_nominal_sampling_rate);

        // Get the ASRC output data
        bytes_received = rtos_intertile_rx_len(
                intertile_i2s_audio_ctx,
                appconfAUDIOPIPELINE_PORT,
                portMAX_DELAY);

        if (bytes_received > 0) {
            xassert(bytes_received <= sizeof(i2s_to_usb_samps_interleaved));

            rtos_intertile_rx_data(
                    intertile_i2s_audio_ctx,
                    i2s_to_usb_samps_interleaved,
                    bytes_received);

            usb_audio_send(&i2s_to_usb_samps_interleaved[0][0], (bytes_received >> 3), 2);
        }

    }
}


void usb_audio_init(rtos_intertile_t *intertile_ctx,
                    unsigned priority)
{
    // Init values
    sampFreq = appconfUSB_AUDIO_SAMPLE_RATE;
    clkValid = 1;

    sampleFreqRng.wNumSubRanges = 1;
    sampleFreqRng.subrange[0].bMin = appconfUSB_AUDIO_SAMPLE_RATE;
    sampleFreqRng.subrange[0].bMax = appconfUSB_AUDIO_SAMPLE_RATE;
    sampleFreqRng.subrange[0].bRes = 0;

    rx_buffer = xStreamBufferCreate(2 * CFG_TUD_AUDIO_FUNC_1_EP_OUT_SW_BUF_SZ, 0);

    /*
     * Note: Given the way that the USB callback notifies usb_audio_out_asrc,
     * the size of this buffer MUST NOT be greater than 2 VFE frames.
     */
    samples_from_host_stream_buf = xStreamBufferCreate(2 * sizeof(samp_t) * USB_TO_I2S_ASRC_BLOCK_LENGTH * CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX,
                                                       0);

    /*
     * Note: The USB callback waits until there are at least 2 VFE frames
     * in this buffer before starting to send to the host, so the size of
     * this buffer MUST be AT LEAST 2 VFE frames.
     */
    samples_to_host_stream_buf_size_bytes = 4 * sizeof(samp_t) * I2S_TO_USB_ASRC_BLOCK_LENGTH * CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX;

    samples_to_host_stream_buf = xStreamBufferCreate(samples_to_host_stream_buf_size_bytes, 0);

    xTaskCreate((TaskFunction_t)usb_audio_out_asrc, "usb_audio_out_asrc", portTASK_STACK_DEPTH(usb_audio_out_asrc), intertile_ctx, priority, &usb_audio_out_asrc_handle);


    // Task for receiving audio from the i2s to usb tile
    xTaskCreate((TaskFunction_t) i2s_to_usb_intertile,
            "i2s_to_usb_intertile",
            RTOS_THREAD_STACK_SIZE(i2s_to_usb_intertile),
            NULL,
            appconfAUDIO_PIPELINE_TASK_PRIORITY,
            NULL);
}
