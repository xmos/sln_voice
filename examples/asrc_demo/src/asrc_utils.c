// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
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
    asrc_init_t *asrc_init_ctx = args;

    for(;;)
    {
        asrc_process_frame_ctx_t *asrc_ctx = NULL;
        (void) rtos_osal_queue_receive(&asrc_init_ctx->asrc_queue, &asrc_ctx, RTOS_OSAL_WAIT_FOREVER);

        unsigned n_samps_out = asrc_process((int *)asrc_ctx->input_samples, (int *)asrc_ctx->output_samples, asrc_ctx->fs_ratio, asrc_init_ctx->asrc_ctrl_ptr);

        (void) rtos_osal_queue_send(&asrc_init_ctx->asrc_ret_queue, &n_samps_out, RTOS_OSAL_WAIT_FOREVER);
    }
}
