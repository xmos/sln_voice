// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#include <stdint.h>

#include "xmath/xmath.h"

// The Wanson libasrengine static library is compiled against an older version of lib_xcore_math.
//  This wrapper function is provided for compatibility
int64_t xs3_vect_s16_dot(
    const int16_t b[],
    const int16_t c[],
    const unsigned length)
{
    return vect_s16_dot(b, c, length);
}