// // Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// // XMOS Public License: Version 1

// #ifndef POWER_STATE_H_
// #define POWER_STATE_H_

// // #include <stdint.h>

// // #include "FreeRTOS.h"
// // #include "rtos_osal.h"

// // // typedef struct {
// // //     float vnr_pred;
// // //     float ema_energy;
// // // } power_data_t;

// /**
//  * Typedef of the low power system states.
//  */
// typedef enum power_state {
//     POWER_STATE_LOW,
//     POWER_STATE_LOW_TO_FULL,
//     POWER_STATE_FULL,
//     POWER_STATE_FULL_TO_LOW
// } power_state_t;

// // typedef struct rtos_low_power_struct rtos_low_power_t;

// // struct rtos_low_power_struct {
// //     TimerHandle_t state_timer;
// //     // qspi_flash_ctx_t ctx;
// //     // size_t flash_size;

// //     // unsigned op_task_priority;
// //     // rtos_osal_thread_t op_task;
// //     // rtos_osal_queue_t op_queue;
// //     // rtos_osal_semaphore_t data_ready;
// //     // rtos_osal_mutex_t mutex;
// //     // volatile int spinlock;
// // };


// // // void power_state_init();
// // // void power_state_set(power_state_t state);
// // // power_state_t power_state_data_add(power_data_t *data);


// // inline uint8_t power_state_timer_expired_get(
// //         rtos_low_power_t *ctx)
// // {
// //     return (uint8_t)xTimerIsTimerActive(ctx->state_timer);
// // }



// // // appconfPOWER_FULL_HOLD_DURATION
// // void power_state_init(
// //         rtos_low_power_t *ctx,
// //         size_t hold_duration_ms);

// #endif /* POWER_STATE_H_ */
