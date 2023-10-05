// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef PI_CONTROL_H
#define PI_CONTROL_H

#include <stdint.h>
#include "avg_buffer_level.h"

#ifdef __cplusplus
 extern "C" {
#endif

uint64_t calc_usb_buffer_based_correction(int32_t nominal_i2s_rate, buffer_calc_state_t *long_term_buf_state, buffer_calc_state_t *short_term_buf_state);

#ifdef __cplusplus
 }
#endif

#endif
