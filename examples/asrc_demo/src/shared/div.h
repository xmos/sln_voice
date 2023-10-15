#ifndef DIV_H
#define DIV_H

#include <stdint.h>
#if __xcore__
#include "xmath/xmath.h"
#else
typedef int exponent_t;
typedef struct {
    int32_t mant;       ///< 32-bit mantissa
    exponent_t exp;     ///< exponent
} float_s32_t;
#endif

float_s32_t float_div(float_s32_t dividend, float_s32_t divisor);
uint64_t float_div_u64_fixed_output_q_format(float_s32_t dividend, float_s32_t divisor, int32_t output_q_format);

#endif
