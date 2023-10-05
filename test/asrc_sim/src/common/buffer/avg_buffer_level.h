// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef AVG_BUFFER_LEVEL_H
#define AVG_BUFFER_LEVEL_H

#include <stdint.h>

#ifdef __cplusplus
 extern "C" {
#endif

typedef struct
{
    int64_t error_accum;
    int32_t avg_buffer_level;
    int32_t prev_avg_buffer_level;
    int32_t stable_avg_level;
    int32_t window_len_log2;
    int32_t buffer_level_stable_threshold; // No. of frames to wait before declaring the buffer level stable
    int32_t count;
    int32_t buffer_level_stable_count;

    bool flag_prev_avg_valid;
    bool flag_first_done;
    bool flag_stable_avg;
}buffer_calc_state_t;

void init_calc_buffer_level_state(buffer_calc_state_t *p_calc_state, int32_t window_len_log2, int32_t buffer_level_stable_threshold);

void calc_avg_buffer_level(buffer_calc_state_t *state, int current_level, bool reset);

#ifdef __cplusplus
 }
 #endif
#endif
