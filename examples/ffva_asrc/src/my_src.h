#pragma once

#include <stdint.h>
#include "app_conf.h"

void stage_upsampler(
    int32_t (*frame_data)[appconfAUDIO_PIPELINE_CHANNELS],
    int32_t (*output)[appconfAUDIO_PIPELINE_CHANNELS]
);

void stage_downsampler(
    int32_t (*input)[appconfAUDIO_PIPELINE_CHANNELS],
    int32_t (*output)[appconfAUDIO_PIPELINE_CHANNELS]
);
