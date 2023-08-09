#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <xcore/chanend.h>
#include <xcore/channel.h>
#include <xcore/hwtimer.h>


#include "defines.h"

/* FreeRTOS headers */

/* App headers */
#include "src.h"

//asrc_process_frame_ctx_t asrc_ctx;

fs_code_t samp_rate_to_code(unsigned samp_rate){
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

void asrc_task(chanend_t c_file_to_asrc, chanend_t c_asrc_to_file)
{
    printf("IN ASRC PROCESS TASK\n");

    // 1 ASRC instance per channel, so 2 for 2 channels. Each ASRC instance processes one channel
    asrc_state_t     asrc_state[ASRC_NUM_INSTANCES][ASRC_CHANNELS_PER_INSTANCE]; //ASRC state machine state
    int              asrc_stack[ASRC_NUM_INSTANCES][ASRC_CHANNELS_PER_INSTANCE][ASRC_STACK_LENGTH_MULT * INPUT_SAMPLES_PER_FRAME]; //Buffer between filter stages
    asrc_ctrl_t      asrc_ctrl[ASRC_NUM_INSTANCES][ASRC_CHANNELS_PER_INSTANCE];  //Control structure
    asrc_adfir_coefs_t asrc_adfir_coefs[ASRC_NUM_INSTANCES];

    for(int ch=0; ch<ASRC_NUM_INSTANCES; ch++)
    {
        for(int ui = 0; ui < ASRC_CHANNELS_PER_INSTANCE; ui++)
        {
            //Set state, stack and coefs into ctrl structure
            asrc_ctrl[ch][ui].psState                   = &asrc_state[ch][ui];
            asrc_ctrl[ch][ui].piStack                   = asrc_stack[ch][ui];
            asrc_ctrl[ch][ui].piADCoefs                 = asrc_adfir_coefs[ch].iASRCADFIRCoefs;
        }
    }
    asrc_init_t asrc_init_ctx;
    asrc_init_ctx.fs_in = INPUT_SAMPLE_RATE; // I2S rate is detected at runtime
    asrc_init_ctx.fs_out = OUTPUT_SAMPLE_RATE;
    asrc_init_ctx.n_in_samples = INPUT_SAMPLES_PER_FRAME;
    asrc_init_ctx.asrc_ctrl_ptr = &asrc_ctrl[1][0];

    fs_code_t in_fs_code = samp_rate_to_code(asrc_init_ctx.fs_in);  //Sample rate code 0..5
    fs_code_t out_fs_code = samp_rate_to_code(asrc_init_ctx.fs_out);
    uint32_t nominal_fs_ratio;
    for(int i=0; i<ASRC_NUM_INSTANCES; i++)
    {
        nominal_fs_ratio = asrc_init(in_fs_code, out_fs_code, &asrc_ctrl[i][0], ASRC_CHANNELS_PER_INSTANCE, asrc_init_ctx.n_in_samples, ASRC_DITHER_SETTING);
    }

    printf("Nominal_fs_ratio = 0x%lx\n",nominal_fs_ratio);

    int32_t tmp[INPUT_SAMPLES_PER_FRAME][NUM_CHANNELS];
    int32_t tmp_deinterleaved[NUM_CHANNELS][INPUT_SAMPLES_PER_FRAME];
    int32_t frame_samples[appconfAUDIO_PIPELINE_CHANNELS][I2S_TO_USB_ASRC_BLOCK_LENGTH*4 + I2S_TO_USB_ASRC_BLOCK_LENGTH];
    int32_t frame_samples_interleaved[I2S_TO_USB_ASRC_BLOCK_LENGTH*4 + I2S_TO_USB_ASRC_BLOCK_LENGTH][appconfAUDIO_PIPELINE_CHANNELS] = {{0}};
    uint32_t max_time = 0;
    float fFsRatioDeviation = 1; //0.990099;
    uint32_t current_rate_ratio = (uint32_t)(nominal_fs_ratio * fFsRatioDeviation); //nominal_fs_ratio;

    //uint32_t current_rate_ratio = 0x3f5dc832; //nominal_fs_ratio;
    //uint32_t current_rate_ratio = 0x40a3d2d8; //nominal_fs_ratio;
    //uint32_t current_rate_ratio =  0x40500000; //nominal_fs_ratio;
    for(;;)
    {
        chan_in_buf_word(c_file_to_asrc, (uint32_t*)tmp, sizeof(tmp)/sizeof(int32_t));
        uint32_t start = get_reference_time();
        if(ASRC_CHANNELS_PER_INSTANCE == 1) // process one channel per instance
        {
            for(int ch=0; ch<appconfAUDIO_PIPELINE_CHANNELS; ch++)
            {
                for(int sample=0; sample<I2S_TO_USB_ASRC_BLOCK_LENGTH; sample++)
                {
                    tmp_deinterleaved[ch][sample] = tmp[sample][ch];
                }
            }
        }
        
        unsigned n_samps_out;
        if(ASRC_CHANNELS_PER_INSTANCE == 1) // process one channel per instance
        {
            n_samps_out = asrc_process((int *)&tmp_deinterleaved[0][0], (int *)&frame_samples[0][0], current_rate_ratio, &asrc_ctrl[0][0]);
            for(int i=0; i<n_samps_out; i++)
            {
                for(int ch=0; ch<1/*appconfAUDIO_PIPELINE_CHANNELS*/; ch++)
                {
                    frame_samples_interleaved[i][ch] = frame_samples[ch][i];               
                }
            }
        }
        else
        {
            n_samps_out = asrc_process((int *)&tmp[0][0], (int *)&frame_samples_interleaved[0][0], current_rate_ratio, &asrc_ctrl[0][0]);            
        }
        uint32_t end = get_reference_time();

        if(max_time < (end - start))
        {
            max_time = end - start;
        }


        chan_out_word(c_file_to_asrc, max_time);
        chan_out_word(c_file_to_asrc, n_samps_out);
        chan_out_buf_word(c_file_to_asrc, (uint32_t*)frame_samples_interleaved, n_samps_out*appconfAUDIO_PIPELINE_CHANNELS);
    }

}
