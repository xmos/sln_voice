// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef LP_CONTROL_H_
#define LP_CONTROL_H_

#include "FreeRTOS.h"
#include "rtos_osal.h"


typedef enum lp_slave_event_group_bits {
    LP_SLAVE_LP_REQ_ACTIVE = 0,
    LP_SLAVE_LP_INF_NOT_ACTIVE,
    LP_SLAVE_LP_INF_PROC_ACTIVE
} lp_slave_event_group_bits_t;

#define LP_ALL_SLAVE_EVENT_BITS     LP_SLAVE_LP_REQ_ACTIVE | LP_SLAVE_LP_INF_NOT_ACTIVE | LP_SLAVE_LP_INF_PROC_ACTIVE

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

power_state_t get_lp_power_state(rtos_low_power_t *ctx);
void lp_master_voice_activity_present(rtos_low_power_t *ctx);


extern rtos_low_power_t *lp_ctx;

#endif /* LP_CONTROL_H_ */