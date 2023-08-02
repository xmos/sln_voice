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
    unsigned nominal_fs_ratio;
}asrc_process_frame_ctx_t;


typedef struct {
    uint32_t fs_in;
    uint32_t fs_out;
    uint32_t n_in_samples;
    asrc_ctrl_t *asrc_ctrl_ptr;
    rtos_osal_queue_t asrc_queue;
    rtos_osal_queue_t asrc_ret_queue;
}asrc_init_t;

fs_code_t samp_rate_to_code(unsigned samp_rate);

#endif
