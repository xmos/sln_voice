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
 * @brief Notify that the power control task should exit the low power state.
 */
void power_control_exit_low_power(void);

/**
 * @brief Get the power control state.
 *
 * @returns The applied power state.
 */
power_state_t power_control_state_get(void);

/**
 * @brief Signal to the power control task that it should halt. This request
 * is only acted on when operating in full power mode and the other tile
 * requests low power. In this case, the application locks the device to
 * full power operation.
 */
void power_control_halt(void);

#else

/**
 * @brief Notify the power control task that the low power state has been
 * requested. The power control task may accept or reject the request.
 */
void power_control_req_low_power(void);

/**
 * @brief Notify the power control task that indication oof the power state
 * has completed, and it is safe to proceed with the requested operation.
 */
void power_control_ind_complete(void);

#endif

#endif /* POWER_CONTROL_H_ */
