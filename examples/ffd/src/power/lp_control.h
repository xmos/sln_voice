// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef LP_CONTROL_H_
#define LP_CONTROL_H_

#include <stdbool.h>

#include "FreeRTOS.h"
#include "rtos_osal.h"

typedef enum lp_slave_event_group_bits {
    /* When 1, that means low power request is active */
    LP_SLAVE_LP_REQ_ACTIVE = 0,
    /* When 1, that means safe to power down */
    LP_SLAVE_LP_INT_HANDLER
} lp_slave_event_group_bits_t;

#define LP_ALL_SLAVE_EVENT_BITS     (1 << LP_SLAVE_LP_REQ_ACTIVE)  \
                                  | (1 << LP_SLAVE_LP_INT_HANDLER)

/**
 * Typedef of the low power system states.
 */
typedef enum power_state {
    POWER_STATE_FULL,
    POWER_STATE_FULL_TO_LOW,
    POWER_STATE_LOW
} power_state_t;

typedef struct rtos_low_power_struct rtos_low_power_t;

struct rtos_low_power_struct {
    TimerHandle_t state_timer;
    power_state_t power_state;
    void *app_data;

    unsigned op_task_priority;
    rtos_osal_thread_t op_task;
    rtos_osal_queue_t op_queue;
    rtos_osal_semaphore_t resp_ready;
    rtos_osal_mutex_t mutex;

    /* Slave only */
    rtos_osal_event_group_t lp_slave_event_group;
    rtos_osal_thread_t lp_slave_power_down_safe;
};

void lp_master_task_create(rtos_low_power_t *ctx, unsigned priority, void *args);
void lp_slave_task_create(rtos_low_power_t *ctx, unsigned priority, void *args);

power_state_t get_lp_power_state_ll(rtos_low_power_t *ctx);
power_state_t get_lp_power_state(rtos_low_power_t *ctx);

void lp_master_voice_activity_present(rtos_low_power_t *ctx);

bool lp_slave_req_active(rtos_low_power_t *ctx);

void lp_slave_user_active(rtos_low_power_t *ctx, lp_slave_event_group_bits_t bitmask);
void lp_slave_user_not_active(rtos_low_power_t *ctx, lp_slave_event_group_bits_t bitmask);

extern rtos_low_power_t *lp_ctx;

#endif /* LP_CONTROL_H_ */