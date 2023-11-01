// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef USB_RATE_CALC_H
#define USB_RATE_CALC_H

#include <stdint.h>

#ifdef __cplusplus
 extern "C" {
#endif

#include "div.h"

typedef struct
{
    uint32_t samples;
    uint32_t ticks;
}rate_info_t;

float_s32_t determine_USB_audio_rate(uint32_t timestamp,
                                    uint32_t data_length,
                                    uint32_t direction,
                                    bool update
                                    );

uint64_t test_mult_div(rate_info_t numerator, rate_info_t denominator);



#ifdef __cplusplus
 }
#endif
#endif
