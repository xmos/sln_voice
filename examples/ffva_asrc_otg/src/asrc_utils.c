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

/* App headers */
#include "app_conf.h"
#include "platform/driver_instances.h"
#include "src.h"
#include "asrc_utils.h"

//Helper function for converting sample to fs index value
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


void asrc_one_channel_task(void *args)
{
    asrc_init_t *asrc_init_ctx = (asrc_init_t*)args;
    uint32_t sampling_rate;
    xTaskNotifyWait(0, ~0, &sampling_rate, portMAX_DELAY);

    printf("In asrc_one_channel_task(), Received sampling rate %lu\n", asrc_init_ctx->fs_in);

    //asrc_init_ctx->fs_in = sampling_rate;
    //Initialise ASRC
    fs_code_t in_fs_code = samp_rate_to_code(asrc_init_ctx->fs_in);  //Sample rate code 0..5
    fs_code_t out_fs_code = samp_rate_to_code(asrc_init_ctx->fs_out);

    unsigned nominal_fs_ratio = asrc_init(in_fs_code, out_fs_code, asrc_init_ctx->asrc_ctrl_ptr, ASRC_CHANNELS_PER_INSTANCE, asrc_init_ctx->n_in_samples, ASRC_DITHER_SETTING);
    printf("Input ASRC: nominal_fs_ratio = %d\n", nominal_fs_ratio);
    unsigned max_ticks = 0;
    for(;;)
    {
        asrc_process_frame_ctx_t *asrc_ctx = NULL;
        (void) rtos_osal_queue_receive(&asrc_init_ctx->asrc_queue, &asrc_ctx, RTOS_OSAL_WAIT_FOREVER);
        unsigned n_samps_out = asrc_process((int *)asrc_ctx->input_samples, (int *)asrc_ctx->output_samples, asrc_ctx->nominal_fs_ratio, asrc_init_ctx->asrc_ctrl_ptr);

        (void) rtos_osal_queue_send(&asrc_init_ctx->asrc_ret_queue, &n_samps_out, RTOS_OSAL_WAIT_FOREVER);
    }
}
