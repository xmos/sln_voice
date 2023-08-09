#ifndef DEFINES_H
#define DEFINES_H

#include "src.h"

#define NUM_CHANNELS (2)
#define INPUT_SAMPLES_PER_FRAME   (256)
#define INPUT_SAMPLE_RATE (48000)
#define OUTPUT_SAMPLE_RATE (192000)
#define ASRC_CHANNELS_PER_INSTANCE (2)
#define ASRC_N_CHANNELS              (2)
#define ASRC_DITHER_SETTING          OFF
#define ASRC_NUM_INSTANCES          (NUM_CHANNELS / ASRC_CHANNELS_PER_INSTANCE)

#define appconfAUDIO_PIPELINE_CHANNELS (NUM_CHANNELS)
#define I2S_TO_USB_ASRC_BLOCK_LENGTH (INPUT_SAMPLES_PER_FRAME)


typedef struct {
    uint32_t fs_in;
    uint32_t fs_out;
    uint32_t n_in_samples;
    asrc_ctrl_t *asrc_ctrl_ptr;
}asrc_init_t;

#endif
