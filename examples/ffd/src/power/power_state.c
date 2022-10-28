// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

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

static TimerHandle_t power_state_timer;
static power_state_t power_state = POWER_STATE_FULL;

void vWakeupStateCallback(TimerHandle_t pxTimer)
{
    power_state_set(POWER_STATE_LOW);
    xTimerStop(power_state_timer, 0);
}

void power_state_set(power_state_t state) {
    if (state != power_state) {
        power_state = state;
        rtos_intertile_tx(intertile_ctx, appconfPOWER_STATE_PORT, &power_state, sizeof(power_state));
    }
}

void power_state_init() {
    power_state_timer = xTimerCreate(
        "disp_reset",
        pdMS_TO_TICKS(appconfPOWER_FULL_HOLD_DURATION),
        pdFALSE,
        NULL,
        vWakeupStateCallback);

    power_state_set(POWER_STATE_LOW);
}

power_state_t power_state_data_add(power_data_t *data) {
    if ((data->ema_energy >= appconfPOWER_HIGH_ENERGY_THRESHOLD) ||
        ((data->ema_energy >= appconfPOWER_LOW_ENERGY_THRESHOLD) &&
         (data->vnr_pred >= appconfPOWER_VNR_THRESHOLD))) {
            power_state_set(POWER_STATE_FULL);
            xTimerReset(power_state_timer, 0);
    }
    
    return power_state;
}
