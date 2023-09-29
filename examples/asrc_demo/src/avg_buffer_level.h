// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef AVG_BUFFER_LEVEL_H
#define AVG_BUFFER_LEVEL_H

#include <stdint.h>

#ifdef __cplusplus
 extern "C" {
#endif

/// @brief Structure containing persistant variables that make up the average buffer level calculation state
typedef struct
{
    int64_t error_accum; /// Accumulated error within an averaging window
    int32_t avg_buffer_level; /// Average buffer level for the latest window
    int32_t stable_avg_level;   /// Stable value of the average wrt which the correction factor is calculated
    int32_t window_len_log2;    /// log2 of the window length in samples over which the buffer level average is calculated
    int32_t buffer_level_stable_threshold; // No. of frames to wait before declaring the buffer level stable
    int32_t count;  /// Running counter for tracking the averaging window
    int32_t buffer_level_stable_count;  /// Frame counter for counting number of averaging windows before declaring that the average is stable

    bool flag_first_done;   /// Flag indicating if the very first windowed average has been computed
    bool flag_stable_avg;   /// Flag indicating whether a stable average has been computed
}buffer_calc_state_t;

/// @brief Initialise an instance of a buffer level calculation state
/// @param p_calc_state                     Pointer to the buffer_calc_state_t state structure
/// @param window_len_log2                  Log2 of the averaging window in samples
/// @param buffer_level_stable_threshold    Threshold indicating the number of windows over which the average is computed before declaring it stable
void init_calc_buffer_level_state(buffer_calc_state_t *p_calc_state, int32_t window_len_log2, int32_t buffer_level_stable_threshold);

/// @brief Calculate the average buffer level.
/// @param state            Pointer to the buffer_calc_state_t state structure
/// @param current_level    Current buffer level
/// @param reset            Flag indicating whether the buffer averaging state needs to reset
void calc_avg_buffer_level(buffer_calc_state_t *state, int current_level, bool reset);

#ifdef __cplusplus
 }
 #endif
#endif
