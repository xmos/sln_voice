// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

/* STD headers */
#include <string.h>
#include <stdint.h>
#include <xcore/hwtimer.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "stream_buffer.h"

/* Library headers */
#include "generic_pipeline.h"

/* App headers */
#include "app_conf.h"
#include "audio_pipeline.h"
#include "audio_pipeline_dsp.h"
#include "platform/driver_instances.h"
#include "my_src.h"
#include "src.h"

#if appconfAUDIO_PIPELINE_FRAME_ADVANCE != 240
#error This pipeline is only configured for 240 frame advance
#endif

typedef struct
{
    /* data */
    rtos_osal_queue_t input_queue;
    rtos_osal_queue_t output_queue;
}pipeline_ctx_t;

static StreamBufferHandle_t input_reference_samples_buf;

static agc_stage_ctx_t DWORD_ALIGNED agc_stage_state = {};

static void stage_agc(frame_data_t *frame_data)
{
#if appconfAUDIO_PIPELINE_SKIP_AGC
#else
    int32_t DWORD_ALIGNED agc_output[appconfAUDIO_PIPELINE_FRAME_ADVANCE];
    configASSERT(AGC_FRAME_ADVANCE == appconfAUDIO_PIPELINE_FRAME_ADVANCE);

    agc_stage_state.md.vnr_flag = frame_data->vnr_pred_flag;
    agc_stage_state.md.aec_ref_power = frame_data->max_ref_energy;
    agc_stage_state.md.aec_corr_factor = frame_data->aec_corr_factor;

    agc_process_frame(
            &agc_stage_state.state,
            agc_output,
            frame_data->samples[0],
            &agc_stage_state.md);
    memcpy(frame_data->samples, agc_output, appconfAUDIO_PIPELINE_FRAME_ADVANCE * sizeof(int32_t));
#endif
}

static void audio_pipeline_input_mic(int32_t **mic_ptr, size_t frame_count)
{
    static int flushed;
    while (!flushed) {
        size_t received;
        received = rtos_mic_array_rx(mic_array_ctx,
                                     mic_ptr,
                                     frame_count,
                                     0);
        if (received == 0) {
            rtos_mic_array_rx(mic_array_ctx,
                              mic_ptr,
                              frame_count,
                              portMAX_DELAY);
            flushed = 1;
        }
    }

    /*
     * NOTE: ALWAYS receive the next frame from the PDM mics,
     * even if USB is the current mic source. The controls the
     * timing since usb_audio_recv() does not block and will
     * receive all zeros if no frame is available yet.
     */
    rtos_mic_array_rx(mic_array_ctx,
                      mic_ptr,
                      frame_count,
                      portMAX_DELAY);
}

static void audio_pipeline_input_i2s(int32_t *i2s_rx_data, size_t frame_count)
{
    size_t rx_count =
    rtos_i2s_rx(i2s_ctx,
                (int32_t*) i2s_rx_data,
                frame_count,
                portMAX_DELAY);
    xassert(rx_count == frame_count);

}

//Helper function for converting sample to fs index value
static fs_code_t samp_rate_to_code(unsigned samp_rate){
    unsigned samp_code = 0xdead;
    switch (samp_rate){
    case 44100:
        samp_code = FS_CODE_44;
        break;
    case 48000:
        samp_code = FS_CODE_48;
        break;
    case 88200:
        samp_code = FS_CODE_88;
        break;
    case 96000:
        samp_code = FS_CODE_96;
        break;
    case 176400:
        samp_code = FS_CODE_176;
        break;
    case 192000:
        samp_code = FS_CODE_192;
        break;
    }
    return samp_code;
}

static inline uint32_t get_clk_count(uint32_t trigger_time, uint32_t prev_trigger_time)
{
    if(trigger_time > prev_trigger_time)
    {
        return trigger_time - prev_trigger_time;
    }
    else
    {
        return ((uint32_t)0xffff - prev_trigger_time + trigger_time);
    }
}

typedef struct
{
    /* data */
    int32_t *input_samples;
    int32_t *output_samples;
    unsigned nominal_fs_ratio;
}asrc_ctx_t;

#define INPUT_ASRC_BLOCK_LENGTH (appconfAUDIO_PIPELINE_FRAME_ADVANCE)
static void asrc_one_channel_task(void *args)
{
    rtos_osal_queue_t *queue = (rtos_osal_queue_t*)args;

    asrc_state_t     asrc_state[ASRC_CHANNELS_PER_INSTANCE]; //ASRC state machine state
    int              asrc_stack[ASRC_CHANNELS_PER_INSTANCE][ASRC_STACK_LENGTH_MULT * INPUT_ASRC_BLOCK_LENGTH]; //Buffer between filter stages
    asrc_ctrl_t      asrc_ctrl[ASRC_CHANNELS_PER_INSTANCE];  //Control structure
    asrc_adfir_coefs_t asrc_adfir_coefs;

    for(int ui = 0; ui < ASRC_CHANNELS_PER_INSTANCE; ui++)
    {
        //Set state, stack and coefs into ctrl structure
        asrc_ctrl[ui].psState                   = &asrc_state[ui];
        asrc_ctrl[ui].piStack                   = asrc_stack[ui];
        asrc_ctrl[ui].piADCoefs                 = asrc_adfir_coefs.iASRCADFIRCoefs;
    }

    //Initialise ASRC
    fs_code_t in_fs_code = samp_rate_to_code(appconfI2S_AUDIO_SAMPLE_RATE);  //Sample rate code 0..5
    fs_code_t out_fs_code = samp_rate_to_code(MIC_ARRAY_SAMPLING_FREQ);
    unsigned nominal_fs_ratio = asrc_init(in_fs_code, out_fs_code, asrc_ctrl, ASRC_CHANNELS_PER_INSTANCE, INPUT_ASRC_BLOCK_LENGTH, ASRC_DITHER_SETTING);
    printf("Input ASRC: nominal_fs_ratio = %d\n", nominal_fs_ratio);

    for(;;)
    {
        asrc_ctx_t *asrc_ctx = NULL;
        (void) rtos_osal_queue_receive(queue, &asrc_ctx, RTOS_OSAL_WAIT_FOREVER);

        unsigned n_samps_out = asrc_process((int *)asrc_ctx->input_samples, (int *)asrc_ctx->output_samples, asrc_ctx->nominal_fs_ratio, asrc_ctrl);

        asrc_ctx->nominal_fs_ratio = n_samps_out; // Reuse nominal_fs_ratio to send back n_samps_out

        (void) rtos_osal_queue_send(queue, &asrc_ctx, RTOS_OSAL_WAIT_FOREVER);
    }
}


static void agc_task(void *args)
{
    pipeline_ctx_t *pipeline_ctx = args;
    const int ref_frame_size_bytes = appconfAUDIO_PIPELINE_FRAME_ADVANCE*sizeof(int32_t); // ONly channel 0 coming from ASRC for now
    static uint32_t prev_in;
    prev_in = get_reference_time();

    for(;;)
    {
        frame_data_t *frame_data;

        frame_data = pvPortMalloc(sizeof(frame_data_t));
        memset(frame_data, 0x00, sizeof(frame_data_t));

        frame_data->vnr_pred_flag = 0;

        audio_pipeline_input_mic((int32_t **)frame_data->mic_samples_passthrough, appconfAUDIO_PIPELINE_FRAME_ADVANCE);

        //At this point check if there's enough data in the reference buffer, otherwise use zeros
        if (xStreamBufferBytesAvailable(input_reference_samples_buf) >= ref_frame_size_bytes)
        {
            //printcharln('m');
            size_t bytes_received = xStreamBufferReceive(input_reference_samples_buf, &frame_data->aec_reference_audio_samples[0][0], ref_frame_size_bytes, 0);
            xassert(bytes_received == ref_frame_size_bytes);
        }
        else
        {
            //printcharln('x');
            memset(&frame_data->aec_reference_audio_samples[0][0], 0, ref_frame_size_bytes);
        }

        //memcpy(frame_data->samples, frame_data->mic_samples_passthrough, sizeof(frame_data->samples));
        memcpy(frame_data->samples, frame_data->aec_reference_audio_samples, sizeof(frame_data->samples)); // For reference passthrough
        //memset(frame_data->samples, 0, sizeof(frame_data->samples));


        // Process
        stage_agc(frame_data);

        uint32_t current_in = get_reference_time();
        //printuintln(current_in - prev_in);
        prev_in = current_in;

        // Send pipeline output
        (void) rtos_osal_queue_send(&pipeline_ctx->output_queue, &frame_data, RTOS_OSAL_WAIT_FOREVER);
    }
}

static void audio_pipeline_input_i(void *args)
{
    printf("SAMPLING_RATE_MULTIPLIER = %d\n", SAMPLING_RATE_MULTIPLIER);
    printf("MIC_ARRAY_SAMPLING_FREQ = %d\n", MIC_ARRAY_SAMPLING_FREQ);

    // Create the 2nd channel ASRC task
    asrc_ctx_t asrc_ctx;
    rtos_osal_queue_t asrc_queue;
    (void) rtos_osal_queue_create(&asrc_queue, "asrc_q", 1, sizeof(asrc_ctx_t*));

    // Create 2nd channel ASRC task
    (void) rtos_osal_thread_create(
        (rtos_osal_thread_t *) NULL,
        (char *) "ASRC_1ch",
        (rtos_osal_entry_function_t) asrc_one_channel_task,
        (void *) (&asrc_queue),
        (size_t) RTOS_THREAD_STACK_SIZE(asrc_one_channel_task),
        (unsigned int) appconfAUDIO_PIPELINE_TASK_PRIORITY);

    rtos_osal_queue_t *pipeline_in_queue = (rtos_osal_queue_t*)args;
    static uint32_t prev_in;
    prev_in = get_reference_time();

    asrc_state_t     asrc_state[ASRC_CHANNELS_PER_INSTANCE]; //ASRC state machine state
    int              asrc_stack[ASRC_CHANNELS_PER_INSTANCE][ASRC_STACK_LENGTH_MULT * INPUT_ASRC_BLOCK_LENGTH]; //Buffer between filter stages
    asrc_ctrl_t      asrc_ctrl[ASRC_CHANNELS_PER_INSTANCE];  //Control structure
    asrc_adfir_coefs_t asrc_adfir_coefs;

    for(int ui = 0; ui < ASRC_CHANNELS_PER_INSTANCE; ui++)
    {
        //Set state, stack and coefs into ctrl structure
        asrc_ctrl[ui].psState                   = &asrc_state[ui];
        asrc_ctrl[ui].piStack                   = asrc_stack[ui];
        asrc_ctrl[ui].piADCoefs                 = asrc_adfir_coefs.iASRCADFIRCoefs;
    }

    //Initialise ASRC
    fs_code_t in_fs_code = samp_rate_to_code(appconfI2S_AUDIO_SAMPLE_RATE);  //Sample rate code 0..5
    fs_code_t out_fs_code = samp_rate_to_code(MIC_ARRAY_SAMPLING_FREQ);
    unsigned nominal_fs_ratio = asrc_init(in_fs_code, out_fs_code, asrc_ctrl, ASRC_CHANNELS_PER_INSTANCE, INPUT_ASRC_BLOCK_LENGTH, ASRC_DITHER_SETTING);
    printf("Input ASRC: nominal_fs_ratio = %d\n", nominal_fs_ratio);

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

    int32_t frame_samples[appconfAUDIO_PIPELINE_CHANNELS][INPUT_ASRC_BLOCK_LENGTH*2];
    for(;;)
    {
        int32_t tmp[INPUT_ASRC_BLOCK_LENGTH][appconfAUDIO_PIPELINE_CHANNELS];
        int32_t tmp_deinterleaved[appconfAUDIO_PIPELINE_CHANNELS][INPUT_ASRC_BLOCK_LENGTH];


        audio_pipeline_input_i2s(&tmp[0][0], INPUT_ASRC_BLOCK_LENGTH); // Receive blocks of INPUT_ASRC_BLOCK_LENGTH at I2S sampling rate
        uint32_t start = get_reference_time();
        for(int ch=0; ch<appconfAUDIO_PIPELINE_CHANNELS; ch++)
        {
            for(int sample=0; sample<INPUT_ASRC_BLOCK_LENGTH; sample++)
            {
                tmp_deinterleaved[ch][sample] = tmp[sample][ch];
            }
        }

        // Send to the other channel ASRC task
        asrc_ctx.input_samples = &tmp_deinterleaved[1][0];
        asrc_ctx.output_samples = &frame_samples[1][0];
        asrc_ctx.nominal_fs_ratio = nominal_fs_ratio;
        asrc_ctx_t *ptr = &asrc_ctx;
        //(void) rtos_osal_queue_send(&asrc_queue, &ptr, RTOS_OSAL_WAIT_FOREVER);
        // Call asrc on this block of samples. Reuse frame_samples now that its copied into aec_reference_audio_samples
        // Only channel 0 for now
        unsigned n_samps_out = asrc_process((int *)&tmp_deinterleaved[0][0], (int *)&frame_samples[0][0], nominal_fs_ratio, asrc_ctrl);

        // Wait for 2nd channel ASRC to finish
        asrc_ctx_t *ctx;
        /*rtos_osal_queue_receive(&asrc_queue, &ctx, RTOS_OSAL_WAIT_FOREVER);
        unsigned n_samps_out_ch1 = ctx->nominal_fs_ratio;
        if(n_samps_out_ch1 != n_samps_out)
        {
            printf("ERROR: n_samps_out %d, n_samps_out_ch1 %d\n", n_samps_out, n_samps_out_ch1);
            xassert(0);
        }*/
        uint32_t end = get_reference_time();
        //printuintln(end - start);
        size_t size_to_write = n_samps_out*sizeof(int32_t); // Do only channel 0 for now
        if (xStreamBufferSpacesAvailable(input_reference_samples_buf) >= size_to_write)
        {
            xStreamBufferSend(input_reference_samples_buf, &frame_samples[0][0], size_to_write, 0);
        }
        else
        {
            printf("Lost I2S samples from host!!\n");
            xassert(0);
        }
    }
}

static int audio_pipeline_output_i(void *args)
{
    asrc_state_t  DWORD_ALIGNED  asrc_state[ASRC_CHANNELS_PER_INSTANCE]; //ASRC state machine state
    int DWORD_ALIGNED asrc_stack[ASRC_CHANNELS_PER_INSTANCE][ASRC_STACK_LENGTH_MULT * OUTPUT_ASRC_N_IN_SAMPLES]; //Buffer between filter stages
    asrc_ctrl_t DWORD_ALIGNED asrc_ctrl[ASRC_CHANNELS_PER_INSTANCE];  //Control structure
    asrc_adfir_coefs_t DWORD_ALIGNED asrc_adfir_coefs;

    for(int ui = 0; ui < ASRC_CHANNELS_PER_INSTANCE; ui++)
    {
        //Set state, stack and coefs into ctrl structure
        asrc_ctrl[ui].psState                   = &asrc_state[ui];
        asrc_ctrl[ui].piStack                   = asrc_stack[ui];
        asrc_ctrl[ui].piADCoefs                 = asrc_adfir_coefs.iASRCADFIRCoefs;
    }

    //Initialise ASRC
    fs_code_t in_fs_code = samp_rate_to_code(MIC_ARRAY_SAMPLING_FREQ); //Sample rate code 0..5
    fs_code_t out_fs_code = samp_rate_to_code(appconfI2S_AUDIO_SAMPLE_RATE);

    unsigned nominal_fs_ratio = asrc_init(in_fs_code, out_fs_code, asrc_ctrl, ASRC_CHANNELS_PER_INSTANCE, OUTPUT_ASRC_N_IN_SAMPLES, ASRC_DITHER_SETTING);
    printf("output ASRC nominal_fs_ratio = %d\n", nominal_fs_ratio);
    rtos_osal_queue_t *queue = (rtos_osal_queue_t*)args;
    for(;;)
    {
        frame_data_t *frame_data;
        (void) rtos_osal_queue_receive(queue, &frame_data, RTOS_OSAL_WAIT_FOREVER);

        // Output ASRC
        uint32_t start = get_reference_time();
        int32_t asrc_output[(OUTPUT_ASRC_N_IN_SAMPLES * 8) + OUTPUT_ASRC_N_IN_SAMPLES]; // TODO calculate this properly
        unsigned n_samps_out = asrc_process((int *)&frame_data->samples[0][0], (int *)asrc_output, nominal_fs_ratio, asrc_ctrl);
        (void)n_samps_out;
        uint32_t end = get_reference_time();
        //printuintln(end - start);
        //printintln(n_samps_out);

        int32_t output[(OUTPUT_ASRC_N_IN_SAMPLES * 8) + OUTPUT_ASRC_N_IN_SAMPLES][appconfAUDIO_PIPELINE_CHANNELS];
        for(int i=0; i<n_samps_out; i++)
        {
            output[i][0] = asrc_output[i];
        }

        rtos_i2s_tx(i2s_ctx,
                (int32_t*) output,
                n_samps_out,
                portMAX_DELAY);

        rtos_osal_free(frame_data);
    }
}


void pipeline_init()
{
#if ON_TILE(1)
    // Initialise one ASRC instance
    agc_init(&agc_stage_state.state, &AGC_PROFILE_FIXED_GAIN);
    agc_stage_state.state.config.gain = f32_to_float_s32(1);

    pipeline_ctx_t *pipeline_ctx = rtos_osal_malloc(sizeof(pipeline_ctx_t));
    (void) rtos_osal_queue_create(&pipeline_ctx->input_queue, NULL, 2, sizeof(void *));
    (void) rtos_osal_queue_create(&pipeline_ctx->output_queue, NULL, 2, sizeof(void *));

    input_reference_samples_buf = xStreamBufferCreate(2 * sizeof(int32_t) * appconfAUDIO_PIPELINE_FRAME_ADVANCE * appconfAUDIO_PIPELINE_CHANNELS,
                                            0);

    // Create pipeline input task
    (void) rtos_osal_thread_create(
        (rtos_osal_thread_t *) NULL,
        (char *) "Pipeline_input",
        (rtos_osal_entry_function_t) audio_pipeline_input_i,
        (void *) (&pipeline_ctx->input_queue),
        (size_t) RTOS_THREAD_STACK_SIZE(audio_pipeline_input_i),
        (unsigned int) appconfAUDIO_PIPELINE_TASK_PRIORITY);

    // Create pipeline output task
    (void) rtos_osal_thread_create(
        (rtos_osal_thread_t *) NULL,
        (char *) "Pipeline_output",
        (rtos_osal_entry_function_t) audio_pipeline_output_i,
        (void *) (&pipeline_ctx->output_queue),
        (size_t) RTOS_THREAD_STACK_SIZE(audio_pipeline_output_i),
        (unsigned int) appconfAUDIO_PIPELINE_TASK_PRIORITY);

    // Create the AGC task
    (void) rtos_osal_thread_create(
        (rtos_osal_thread_t *) NULL,
        (char *) "AGC",
        (rtos_osal_entry_function_t) agc_task,
        (void *) pipeline_ctx,
        (size_t) RTOS_THREAD_STACK_SIZE(agc_task),
        (unsigned int) appconfAUDIO_PIPELINE_TASK_PRIORITY);
#endif
}
