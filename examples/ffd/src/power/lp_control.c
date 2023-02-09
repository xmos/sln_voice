// Copyright (c) 2023 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

/* System headers */
#include <platform.h>
#include <xs1.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"

/* Library headers */
#include "rtos_macros.h"
#include "rtos_clock_control.h"

/* App headers */
#include "app_conf.h"
#include "platform/driver_instances.h"
#include "power/lp_control.h"
#include "gpio_ctrl/leds.h"

static unsigned tile0_div = 0;
static unsigned switch_div = 0;

typedef enum lp_event {
    NO_VOICE_TIMEOUT,
    RESET_VOICE_TIMEOUT,
    GET_LP_STATE,
    MASTER_REQ_SLAVE_LP,
    MASTER_REQ_CANCEL_REQ_SLAVE_LP,
    MASTER_REQ_SLAVE_WAKEUP,
    SLAVE_REQ_SAFE_TO_POWER_DOWN,
} lp_event_t;

typedef struct {
    lp_event_t event;
    uint8_t *data;
    size_t len;
} lp_op_req_t;

static void request(
        rtos_low_power_t *ctx,
        lp_op_req_t *op) {
    rtos_osal_mutex_get(&ctx->mutex, RTOS_OSAL_WAIT_FOREVER);
    rtos_osal_queue_send(&ctx->op_queue, op, RTOS_OSAL_WAIT_FOREVER);
    rtos_osal_mutex_put(&ctx->mutex);
}

void lp_master_voice_activity_present(rtos_low_power_t *ctx) {
    lp_op_req_t op;

    op.event = RESET_VOICE_TIMEOUT;
    request(ctx, &op);
}

static void tile0_power_down(void) {
    rtos_printf("Entering low power...\n");
    // Save clock divider config before apply low power configuration.
    tile0_div = rtos_clock_control_get_processor_clk_div(cc_ctx_t0);
    rtos_clock_control_set_processor_clk_div(cc_ctx_t0, appconfLOW_POWER_OTHER_TILE_CLK_DIV);

#if (appconfLOW_POWER_SWITCH_CLK_DIV_ENABLE)
    switch_div = rtos_clock_control_get_switch_clk_div(cc_ctx_t0);
    rtos_clock_control_set_switch_clk_div(cc_ctx_t0, appconfLOW_POWER_SWITCH_CLK_DIV);
#endif
}

static void tile0_power_up(void) {
    rtos_printf("Entering full power...\n");
    // Restore the original clock divider state(s).
#if (appconfLOW_POWER_ENABLE_SWITCH_CONTROL)
    set_node_switch_clk_div(TILE_ID(0), switch_div);
#endif
    set_tile_processor_clk_div(TILE_ID(0), tile0_div);
}

bool lp_slave_req_active(rtos_low_power_t *ctx) {
    uint32_t flags;

    flags = xEventGroupGetBits((EventGroupHandle_t)&ctx->lp_slave_event_group);
    return flags & (1 << LP_SLAVE_LP_REQ_ACTIVE);
}

void lp_slave_user_active(rtos_low_power_t *ctx, lp_slave_event_group_bits_t bitmask) {
    rtos_printf("%d active\n", bitmask);
    if (rtos_osal_event_group_clear_bits(
            &ctx->lp_slave_event_group,
            (1 << bitmask))
        != RTOS_OSAL_SUCCESS)
    {
        xassert(0); /* This shouldn't ever fail */
    }
}

void lp_slave_user_not_active(rtos_low_power_t *ctx, lp_slave_event_group_bits_t bitmask) {
    rtos_printf("%d not active\n", bitmask);
    if (rtos_osal_event_group_set_bits(
            &ctx->lp_slave_event_group,
            (1 << bitmask))
        != RTOS_OSAL_SUCCESS)
    {
        xassert(0); /* This shouldn't ever fail */
    }
}

power_state_t get_lp_power_state_ll(rtos_low_power_t *ctx) {
    return ctx->power_state;
}

power_state_t get_lp_power_state(rtos_low_power_t *ctx) {
    power_state_t ret;
    lp_op_req_t op;

    op.event = GET_LP_STATE;
    op.data = (uint8_t *)&ret;
    op.len = sizeof(power_state_t);

    request(ctx, &op);

    rtos_osal_semaphore_get(&ctx->resp_ready, RTOS_OSAL_WAIT_FOREVER);
}

static void lp_master_op_thread(rtos_low_power_t *ctx) {
    lp_op_req_t op;

    for(;;) {
        rtos_osal_queue_receive(&ctx->op_queue, &op, RTOS_OSAL_WAIT_FOREVER);

        // rtos_printf("master op got %d\n", op.event);
        switch(op.event) {
            case NO_VOICE_TIMEOUT:
                ctx->power_state = POWER_STATE_FULL_TO_LOW;
                memset(&op, 0x00, sizeof(lp_op_req_t));
                op.event = MASTER_REQ_SLAVE_LP;
                rtos_intertile_tx(intertile_ctx,
                                appconfPOWER_CONTROL_PORT,
                                &op,
                                sizeof(lp_op_req_t));
                break;
            case GET_LP_STATE:
                memcpy(&op.data, &ctx->power_state, op.len);
                rtos_osal_semaphore_put(&ctx->resp_ready);
                break;
            case RESET_VOICE_TIMEOUT:
                switch(ctx->power_state) {
                    case POWER_STATE_FULL:
                        break;
                    case POWER_STATE_FULL_TO_LOW:
                        memset(&op, 0x00, sizeof(lp_op_req_t));
                        op.event = MASTER_REQ_CANCEL_REQ_SLAVE_LP;
                        rtos_intertile_tx(intertile_ctx,
                                        appconfPOWER_CONTROL_PORT,
                                        &op,
                                        sizeof(lp_op_req_t));
                        ctx->power_state = POWER_STATE_FULL;
                        break;
                    case POWER_STATE_LOW:
                        tile0_power_up();
                        memset(&op, 0x00, sizeof(lp_op_req_t));
                        op.event = MASTER_REQ_SLAVE_WAKEUP;
                        rtos_intertile_tx(intertile_ctx,
                                        appconfPOWER_CONTROL_PORT,
                                        &op,
                                        sizeof(lp_op_req_t));
                        ctx->power_state = POWER_STATE_FULL;
                        break;
                    default:
                        configASSERT(0);
                        break;
                }
                xTimerReset(ctx->state_timer, 0);
                break;
            case SLAVE_REQ_SAFE_TO_POWER_DOWN:
                /* Verify this request was not cancelled in
                 * the time it took for tile 0 to enter a safe state */
                if (ctx->power_state == POWER_STATE_FULL_TO_LOW) {
                    xTimerStop(ctx->state_timer, 0);
                    ctx->power_state = POWER_STATE_LOW;
                    tile0_power_down();
                }
                break;
            default:
                rtos_printf("unexpected master op %d\n", op.event);
                break;
        }
    }
}

static void lp_remote_tile_handler(rtos_low_power_t *ctx) {
    lp_op_req_t op;

    while(1) {
        size_t len_rx = rtos_intertile_rx_len(
                                intertile_ctx,
                                appconfPOWER_CONTROL_PORT,
                                RTOS_OSAL_WAIT_FOREVER);

        configASSERT(len_rx == sizeof(lp_op_req_t));              

        rtos_intertile_rx_data(
                        intertile_ctx,
                        &op,
                        len_rx);

        request(ctx, &op);
    }
}

static void lp_slave_power_down_safe(rtos_low_power_t *ctx) {
    lp_op_req_t op;
    rtos_osal_status_t status;
    uint32_t flags;

    op.event = SLAVE_REQ_SAFE_TO_POWER_DOWN;

    for(;;) {
        status = rtos_osal_event_group_get_bits(
                        &ctx->lp_slave_event_group, 
                        LP_ALL_SLAVE_EVENT_BITS,
                        RTOS_OSAL_AND_CLEAR,
                        &flags,
                        RTOS_OSAL_WAIT_FOREVER);

        if (status == RTOS_OSAL_SUCCESS) {
            led_indicate_asleep();
            /* Tell host it is safe to clock us down */
            rtos_intertile_tx(intertile_ctx,
                            appconfPOWER_CONTROL_PORT,
                            &op,
                            sizeof(lp_op_req_t));
        }
    }
}

static void lp_slave_op_thread(rtos_low_power_t *ctx) {
    lp_op_req_t op;

    for(;;) {
        rtos_osal_queue_receive(&ctx->op_queue, &op, RTOS_OSAL_WAIT_FOREVER);

        // rtos_printf("slave op got %d\n", op.event);
        switch(op.event) {
            case MASTER_REQ_SLAVE_LP:
                rtos_printf("%d active\n", LP_SLAVE_LP_REQ_ACTIVE);
                if (rtos_osal_event_group_set_bits(
                        &ctx->lp_slave_event_group,
                        (1 << LP_SLAVE_LP_REQ_ACTIVE))
                    != RTOS_OSAL_SUCCESS)
                {
                    xassert(0); /* This shouldn't ever fail */
                }
                break;
            case MASTER_REQ_SLAVE_WAKEUP:
                led_indicate_awake();
                /* Fallthrough */
            case MASTER_REQ_CANCEL_REQ_SLAVE_LP:
                rtos_printf("%d not active\n", LP_SLAVE_LP_REQ_ACTIVE);
                if (rtos_osal_event_group_clear_bits(
                        &ctx->lp_slave_event_group,
                        (1 << LP_SLAVE_LP_REQ_ACTIVE))
                    != RTOS_OSAL_SUCCESS)
                {
                    xassert(0); /* This shouldn't ever fail */
                }
                break;
            default:
                rtos_printf("unexpected slave op %d\n", op.event);
                break;
        }
    }
}

void vPowerStateTimerCallback(TimerHandle_t pxTimer)
{
    lp_op_req_t op;

    op.event = NO_VOICE_TIMEOUT;

    rtos_low_power_t *ctx = (rtos_low_power_t *)pvTimerGetTimerID(pxTimer);

    if ( rtos_osal_queue_send(&ctx->op_queue, &op, 0) != RTOS_OSAL_SUCCESS) {
        rtos_printf("PowerState Timer event lost\n");
    }

    xTimerStop(pxTimer, 0);
}

void lp_master_task_create(rtos_low_power_t *ctx, unsigned priority, void *args)
{
    rtos_osal_mutex_create(&ctx->mutex, "lp_lock", RTOS_OSAL_RECURSIVE);
    rtos_osal_semaphore_create(&ctx->resp_ready, "lp_resp_sem", 1, 0);
    rtos_osal_queue_create(&ctx->op_queue, "lp_req_queue", 5, sizeof(lp_op_req_t));

    ctx->power_state = POWER_STATE_FULL;

    rtos_osal_thread_create(
            NULL,
            "lp_master_slave_handler",
            (rtos_osal_entry_function_t) lp_remote_tile_handler,
            ctx,
            RTOS_THREAD_STACK_SIZE(lp_remote_tile_handler),
            priority);

    rtos_osal_thread_create(
            &ctx->op_task,
            "lp_master_op_thread",
            (rtos_osal_entry_function_t) lp_master_op_thread,
            ctx,
            RTOS_THREAD_STACK_SIZE(lp_master_op_thread),
            priority);

    ctx->state_timer = xTimerCreate(
            "lp_state_tmr",
            pdMS_TO_TICKS(appconfPOWER_FULL_HOLD_DURATION),
            pdFALSE,
            ctx,
            vPowerStateTimerCallback);

    xTimerStart(ctx->state_timer, 0);
}

void lp_slave_task_create(rtos_low_power_t *ctx, unsigned priority, void *args)
{
    rtos_osal_mutex_create(&ctx->mutex, "lp_lock", RTOS_OSAL_RECURSIVE);
    rtos_osal_semaphore_create(&ctx->resp_ready, "lp_resp_sem", 1, 0);
    rtos_osal_queue_create(&ctx->op_queue, "lp_req_queue", 5, sizeof(lp_op_req_t));
    rtos_osal_event_group_create(&ctx->lp_slave_event_group, "lp_slave_group");

    rtos_osal_thread_create(
            &ctx->lp_slave_power_down_safe,
            "lp_slave_power_down_safe",
            (rtos_osal_entry_function_t) lp_slave_power_down_safe,
            ctx,
            RTOS_THREAD_STACK_SIZE(lp_slave_power_down_safe),
            priority);

    rtos_osal_thread_create(
            NULL,
            "lp_slave_master_handler",
            (rtos_osal_entry_function_t) lp_remote_tile_handler,
            ctx,
            RTOS_THREAD_STACK_SIZE(lp_remote_tile_handler),
            priority);

    rtos_osal_thread_create(
            &ctx->op_task,
            "lp_slave_op_thread",
            (rtos_osal_entry_function_t) lp_slave_op_thread,
            ctx,
            RTOS_THREAD_STACK_SIZE(lp_slave_op_thread),
            priority);
}
