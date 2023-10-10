// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef USB_RATE_CALC_H
#define USB_RATE_CALC_H

#include <stdint.h>

#ifdef __cplusplus
 extern "C" {
#endif
typedef struct
{
    int32_t mant;
    int32_t exp;
}float_s32_t;

typedef struct
{
    uint64_t mant;
    int32_t exp;
}float_u64_t;

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

float_s32_t float_div(float_s32_t dividend, float_s32_t divisor);

uint32_t float_div_fixed_output_q_format(float_s32_t dividend, float_s32_t divisor, int32_t output_q_format);

uint64_t float_div_u64_fixed_output_q_format(float_s32_t dividend, float_s32_t divisor, int32_t output_q_format);

uint64_t test_mult_div(rate_info_t numerator, rate_info_t denominator);



#ifdef __cplusplus
 }
#endif
#endif
