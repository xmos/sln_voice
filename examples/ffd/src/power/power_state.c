// // Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// // XMOS Public License: Version 1

// /* STD headers */
// #include <platform.h>
// #include <xs1.h>

// /* FreeRTOS headers */
// #include "FreeRTOS.h"
// #include "task.h"

// /* App headers */
// #include "app_conf.h"
// #include "platform/driver_instances.h"
// #include "power/power_state.h"
// #include "power/power_control.h"

// #if ON_TILE(POWER_CONTROL_TILE_NO)

// static TimerHandle_t power_state_timer;
// static power_state_t power_state = POWER_STATE_FULL;

// void vPowerStateTimerCallback(TimerHandle_t pxTimer)
// {
//     xTimerStop(power_state_timer, 0);
// }

// void power_state_set(power_state_t state) {
//     if (state != power_state) {
//         power_state = state;

// #if appconfLOW_POWER_ENABLED
//         if (power_state == POWER_STATE_FULL) {
//             power_control_exit_low_power();
//         }
// #endif
//     }
// }

// void power_state_init(rtos_low_power_t *ctx, size_t hold_duration_ms) {
//     ctx->state_timer = xTimerCreate(
//         "pwr_state_tmr",
//         pdMS_TO_TICKS(hold_duration_ms),
//         pdFALSE,
//         NULL,
//         vPowerStateTimerCallback);

//     xTimerStart(ctx->state_timer, 0);
// }

// power_state_t power_state_data_add(power_data_t *data) {
//     if ((data->ema_energy >= appconfPOWER_HIGH_ENERGY_THRESHOLD) ||
//         ((data->ema_energy >= appconfPOWER_LOW_ENERGY_THRESHOLD) &&
//          (data->vnr_pred >= appconfPOWER_VNR_THRESHOLD))) {
//             power_state_set(POWER_STATE_FULL);
//             xTimerReset(power_state_timer, 0);
//     }

//     return power_state;
// }

// uint8_t power_state_timer_expired_get(void)
// {
//     return (uint8_t)xTimerIsTimerActive(power_state_timer);
// }

// #endif // ON_TILE(POWER_CONTROL_TILE_NO)