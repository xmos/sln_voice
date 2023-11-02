// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef DIV_H
#define DIV_H

#include <stdint.h>

// This file contains functions shared between the ASRC example application and the ASRC simulator code

#if XCORE_MATH_NOT_INCLUDED // Simulator does not include lib_xcore_math
    typedef int exponent_t;
    typedef struct {
        int32_t mant;       ///< 32-bit mantissa
        exponent_t exp;     ///< exponent
    } float_s32_t;
#else
    #include "xmath/xmath.h"
#endif

/**
 * @brief Floating point division for unsigned numbers.
 *
 * This function assumes the dividend and divisor and unsigned and due to this assumption, results in a
 * couple of extra bits of precision when compared to the lib_xcore_math float_s32_div function.
 *
 * @param dividend
 * @param divisor
 * @return float_s32_t result of the division
 */
float_s32_t float_div(float_s32_t dividend, float_s32_t divisor);

/**
 * @brief Floating point division for unsigned numbers where the Q-format of the output is fixed.
 *
 * @param dividend
 * @param divisor
 * @param output_q_format Q format of the output. for example, if the output is desired in Q60 format, set output_q_format to 60
 * @return uint64_t 64 bit mantissa of the division result. The double precision output would be result_mantissa * pow(2, -output_q_format)
 */
uint64_t float_div_u64_fixed_output_q_format(float_s32_t dividend, float_s32_t divisor, int32_t output_q_format);

#endif
