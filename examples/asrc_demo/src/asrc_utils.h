// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef ASRC_UTILS_H
#define ASRC_UTILS_H

#include <stdint.h>
/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "queue.h"
#include "src.h"

typedef struct
{
    /* data */
    int32_t *input_samples;
    int32_t *output_samples;
    uint64_t fs_ratio;
    unsigned i2s_sampling_rate;
}asrc_process_frame_ctx_t;


typedef struct {
    uint32_t fs_in;
    uint32_t fs_out;
    uint32_t n_in_samples;
    asrc_ctrl_t *asrc_ctrl_ptr;
    rtos_osal_queue_t asrc_queue;
    rtos_osal_queue_t asrc_ret_queue;
}asrc_init_t;

#define USB_TO_I2S_ASRC_BLOCK_LENGTH (96)
#define I2S_TO_USB_ASRC_BLOCK_LENGTH (244)  // Found out from simulation. Relatively jitter free average buffer levels seen with 244 samples block than 240 samples block size
#define ASRC_N_CHANNELS              (1)
#define ASRC_CHANNELS_PER_INSTANCE   (1)
#define ASRC_DITHER_SETTING          OFF

fs_code_t samp_rate_to_code(unsigned samp_rate);
void asrc_one_channel_task(void *args);

#endif
