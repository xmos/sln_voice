// Copyright (c) 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public License: Version 1

/* STD headers */
#include <platform.h>
#include <xs1.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"

/* App headers */
#include "app_conf.h"
#include "platform/driver_instances.h"
#include "power/power_state.h"
#include "power/power_control.h"

#if ON_TILE(POWER_CONTROL_TILE_NO)

static TimerHandle_t power_state_timer;
static power_state_t power_state = POWER_STATE_FULL;
static uint8_t power_state_time_expired = 1;

void vPowerStateTimerCallback(TimerHandle_t pxTimer)
{
    power_state_time_expired = 1;
    xTimerStop(power_state_timer, 0);
}

void power_state_set(power_state_t state) {
    if (state != power_state) {
        power_state = state;

        if (power_state == POWER_STATE_FULL)
            power_control_exit_low_power();
    }
}

void power_state_init() {
    power_state_timer = xTimerCreate(
        "pwr_state_tmr",
        pdMS_TO_TICKS(appconfPOWER_FULL_HOLD_DURATION),
        pdFALSE,
        NULL,
        vPowerStateTimerCallback);

    power_state_time_expired = 0;
    xTimerStart(power_state_timer, 0);
}

power_state_t power_state_data_add(power_data_t *data) {
    if ((data->ema_energy >= appconfPOWER_HIGH_ENERGY_THRESHOLD) ||
        ((data->ema_energy >= appconfPOWER_LOW_ENERGY_THRESHOLD) &&
         (data->vnr_pred >= appconfPOWER_VNR_THRESHOLD))) {
            power_state_set(POWER_STATE_FULL);
            power_state_time_expired = 0;
            xTimerReset(power_state_timer, 0);
    }

    return power_state;
}

uint8_t power_state_timer_expired_get(void)
{
    return power_state_time_expired;
}

#endif // ON_TILE(POWER_CONTROL_TILE_NO)
