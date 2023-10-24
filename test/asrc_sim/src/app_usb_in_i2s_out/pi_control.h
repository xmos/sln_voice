// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef PI_CONTROL_H
#define PI_CONTROL_H

#include <stdint.h>
#include "avg_buffer_level.h"

#ifdef __cplusplus
 extern "C" {
#endif

void calc_avg_i2s_send_buffer_level(int current_level, bool reset);
uint64_t pi_control(int32_t nominal_i2s_rate, buffer_calc_state_t *buf_state);

#ifdef __cplusplus
 }
#endif

#endif
