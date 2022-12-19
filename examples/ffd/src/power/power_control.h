// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef POWER_CONTROL_H_
#define POWER_CONTROL_H_

#include "app_conf.h"
#include "power_state.h"

// Specifies the tile that is controlling the low power mode.
#define POWER_CONTROL_TILE_NO        AUDIO_PIPELINE_TILE_NO

/**
 * @brief Initialize the power control task.
 *
 * @param priority The priority of the task.
 * @param args The arguments to send to the task.
 */
void power_control_task_create(unsigned priority, void *args);

#if ON_TILE(POWER_CONTROL_TILE_NO)

/**
 * @brief Notify that the power control task should enter the low power state.
 */
void power_control_enter_low_power(void);

/**
 * @brief Notify that the power control task should exit the low power state.
 */
void power_control_exit_low_power(void);

/**
 * @brief Get the power control state.
 *
 * @returns The applied power state.
 */
power_state_t power_control_state_get(void);

#else

/**
 * @brief Notify that the requested power control state on tile[0] has completed
 * This serves control when tile[1] is allowed to commence with applying
 * low power mode.
 */
void power_control_req_complete(void);

#endif

#endif /* POWER_CONTROL_H_ */