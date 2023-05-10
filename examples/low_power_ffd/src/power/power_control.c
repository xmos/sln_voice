// Copyright (c) 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public License: Version 1

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
#include "gpio_ctrl/leds.h"
#include "power/power_state.h"
#include "power/power_control.h"
#include "intent_engine.h"

#ifndef DEBUG_LOW_POWER_TASK
#define DEBUG_LOW_POWER_TASK             0
#endif

#define TASK_NOTIF_MASK_LP_ENTER         1  // Used by tile: !POWER_CONTROL_TILE_NO
#define TASK_NOTIF_MASK_LP_EXIT          2  // Used by tile: POWER_CONTROL_TILE_NO
#define TASK_NOTIF_MASK_LP_IND_COMPLETE  4  // Used by tile: !POWER_CONTROL_TILE_NO

// States of the power control task.
typedef enum power_control_state {
    PWR_CTRL_STATE_LOW_POWER_REQUEST,
    PWR_CTRL_STATE_LOW_POWER_RESPONSE,
    PWR_CTRL_STATE_LOW_POWER_READY,
    PWR_CTRL_STATE_FULL_POWER
} power_control_state_t;

static const uint32_t bits_to_clear_on_entry = 0x00000000UL;
static const uint32_t bits_to_clear_on_exit = 0xFFFFFFFFUL;

static TaskHandle_t ctx_power_control_task = NULL;

#if ON_TILE(POWER_CONTROL_TILE_NO)

static power_state_t power_state = POWER_STATE_FULL;
static unsigned tile0_div;
static unsigned switch_div;

#endif

static void driver_control_lock(void)
{
#if ON_TILE(POWER_CONTROL_TILE_NO)
    rtos_osal_mutex_get(&gpio_ctx_t0->lock, RTOS_OSAL_WAIT_FOREVER);
#else
    rtos_osal_mutex_get(&qspi_flash_ctx->mutex, RTOS_OSAL_WAIT_FOREVER);
    rtos_osal_mutex_get(&i2c_master_ctx->lock, RTOS_OSAL_WAIT_FOREVER);
    rtos_osal_mutex_get(&uart_tx_ctx->lock, RTOS_OSAL_WAIT_FOREVER);
#endif
}

static void driver_control_unlock(void)
{
#if ON_TILE(POWER_CONTROL_TILE_NO)
    rtos_osal_mutex_put(&gpio_ctx_t0->lock);
#else
    rtos_osal_mutex_put(&uart_tx_ctx->lock);
    rtos_osal_mutex_put(&i2c_master_ctx->lock);
    rtos_osal_mutex_put(&qspi_flash_ctx->mutex);
#endif
}

#if ON_TILE(POWER_CONTROL_TILE_NO)

static void low_power_clocks_enable(void)
{
    // Save clock divider config before apply low power configuration.
    tile0_div = rtos_clock_control_get_processor_clk_div(cc_ctx_t0);
    rtos_clock_control_set_processor_clk_div(cc_ctx_t0, appconfLOW_POWER_OTHER_TILE_CLK_DIV);

#if (appconfLOW_POWER_SWITCH_CLK_DIV_ENABLE)
    switch_div = rtos_clock_control_get_switch_clk_div(cc_ctx_t0);
    rtos_clock_control_set_switch_clk_div(cc_ctx_t0, appconfLOW_POWER_SWITCH_CLK_DIV);
#endif
}

static void low_power_clocks_disable(void)
{
    // Restore the original clock divider state(s).
#if (appconfLOW_POWER_ENABLE_SWITCH_CONTROL)
    set_node_switch_clk_div(TILE_ID(0), switch_div);
#endif
    set_tile_processor_clk_div(TILE_ID(0), tile0_div);
}

#endif /* ON_TILE(POWER_CONTROL_TILE_NO) */

static void low_power_request(void)
{
    power_state_t requested_power_state;

#if ON_TILE(POWER_CONTROL_TILE_NO)
    /*
     * Wait for other tile to request low power mode.
     */
    size_t len_rx = rtos_intertile_rx_len(intertile_ctx,
                                          appconfPOWER_CONTROL_PORT,
                                          RTOS_OSAL_WAIT_FOREVER);
    configASSERT(len_rx == sizeof(requested_power_state));

    rtos_intertile_rx_data(intertile_ctx,
                           &requested_power_state,
                           sizeof(requested_power_state));
    configASSERT(requested_power_state == POWER_STATE_LOW);
#else
    uint32_t notif_value;

    /*
     * Wait for a notification, signaling to enter low power mode.
     */
    xTaskNotifyWait(bits_to_clear_on_entry,
                    bits_to_clear_on_exit,
                    &notif_value,
                    portMAX_DELAY);
    configASSERT(notif_value == TASK_NOTIF_MASK_LP_ENTER);

    /*
     * Send a low power request to the other tile.
     */
    requested_power_state = POWER_STATE_LOW;
    rtos_intertile_tx(intertile_ctx,
                      appconfPOWER_CONTROL_PORT,
                      &requested_power_state,
                      sizeof(requested_power_state));
#endif
}

static uint8_t low_power_response(void)
{
    uint8_t full_pwr_time_expired;

#if ON_TILE(POWER_CONTROL_TILE_NO)
    /*
     * Send an ACK/NAK based on whether the power state timer has expired.
     */
    full_pwr_time_expired = power_state_timer_expired_get();

    /* The power state is updated during the response instead of during
     * "low_power_ready()" in order to avoid reports of "lost output samples
     * for inference" */
    power_state = (full_pwr_time_expired) ?
        POWER_STATE_LOW :
        POWER_STATE_FULL;

    rtos_intertile_tx(intertile_ctx,
                      appconfPOWER_CONTROL_PORT,
                      &full_pwr_time_expired,
                      sizeof(full_pwr_time_expired));
#else
    /*
     * Wait for ACK/NAK, based on whether the full power timer has elapsed.
     */
    size_t len_rx = rtos_intertile_rx_len(intertile_ctx,
                                          appconfPOWER_CONTROL_PORT,
                                          RTOS_OSAL_WAIT_FOREVER);
    configASSERT(len_rx == sizeof(full_pwr_time_expired));

    rtos_intertile_rx_data(intertile_ctx,
                           &full_pwr_time_expired,
                           sizeof(full_pwr_time_expired));

    if (full_pwr_time_expired == 0) {
        // Timer has not expired, NAK the request to continue in full power.
        intent_engine_full_power_request();
    } else {
        debug_printf("Entering low power...\n");
        intent_engine_low_power_accept();
    }
#endif

    return full_pwr_time_expired;
}

static void low_power_ready(void)
{
    uint8_t low_pwr_ready;

#if ON_TILE(POWER_CONTROL_TILE_NO)
    /*
     * Wait for other tile to indicate ready for low power.
     * Currently the received value is not used/important (but should be set to 1).
     */
    size_t len_rx = rtos_intertile_rx_len(intertile_ctx,
                                          appconfPOWER_CONTROL_PORT,
                                          RTOS_OSAL_WAIT_FOREVER);
    configASSERT(len_rx == sizeof(low_pwr_ready));

    rtos_intertile_rx_data(intertile_ctx, &low_pwr_ready, sizeof(low_pwr_ready));
    configASSERT(low_pwr_ready == 1);

    power_state_set(power_state);
    driver_control_lock();
    low_power_clocks_enable();
    debug_printf("Entered low power.\n");
#else
    uint32_t notif_value;

    /*
     * Update power state indicators and wait for a notification, signaling
     * that the LED indication has been applied and that the tile is ready
     * to be set to low power mode by the other tile.
     */
    led_indicate_asleep();
    xTaskNotifyWait(bits_to_clear_on_entry,
                    bits_to_clear_on_exit,
                    &notif_value,
                    portMAX_DELAY);
    configASSERT(notif_value == TASK_NOTIF_MASK_LP_IND_COMPLETE);

    driver_control_lock();
    intent_engine_keyword_queue_reset();

    /*
     * Signal to the other tile that it is ready to enter low power mode.
     */
    low_pwr_ready = 1;
    rtos_intertile_tx(intertile_ctx,
                        appconfPOWER_CONTROL_PORT,
                        &low_pwr_ready,
                        sizeof(low_pwr_ready));
#endif
}

static void full_power(void)
{
    power_state_t requested_power_state;
    uint32_t notif_value;

#if ON_TILE(POWER_CONTROL_TILE_NO)
    /*
     * Wait for notification to return to POWER_STATE_FULL.
     */
    xTaskNotifyWait(bits_to_clear_on_entry,
                    bits_to_clear_on_exit,
                    &notif_value,
                    portMAX_DELAY);

    configASSERT(notif_value == TASK_NOTIF_MASK_LP_EXIT);
    requested_power_state = POWER_STATE_FULL;

    debug_printf("Exiting low power...\n");

    low_power_clocks_disable();
    driver_control_unlock();

    /*
     * Notify other tile of state change; and begin full power operation.
     */
    rtos_intertile_tx(intertile_ctx,
                    appconfPOWER_CONTROL_PORT,
                    &requested_power_state,
                    sizeof(requested_power_state));

    power_state = POWER_STATE_FULL;
    debug_printf("Exited low power.\n");
#else
    /*
     * Wait for indication from other tile indicating that it should return
     * to full power mode.
     */
    size_t len_rx = rtos_intertile_rx_len(intertile_ctx,
                                          appconfPOWER_CONTROL_PORT,
                                          RTOS_OSAL_WAIT_FOREVER);
    configASSERT(len_rx == sizeof(requested_power_state));

    rtos_intertile_rx_data(intertile_ctx,
                           &requested_power_state,
                           sizeof(requested_power_state));
    configASSERT(requested_power_state == POWER_STATE_FULL);

    driver_control_unlock();
    led_indicate_awake();

    /*
     * Wait for a notification, signaling that the LED indication has
     * been applied.
     */
    xTaskNotifyWait(bits_to_clear_on_entry,
                    bits_to_clear_on_exit,
                    &notif_value,
                    portMAX_DELAY);
    configASSERT(notif_value == TASK_NOTIF_MASK_LP_IND_COMPLETE);

    // Restart the timer for holding full power.
    intent_engine_full_power_request();
#endif
}

static void power_control_task(void *arg)
{
    power_control_state_t state = PWR_CTRL_STATE_LOW_POWER_REQUEST;

#if DEBUG_LOW_POWER_TASK
    debug_printf("Starting power_control_task() on tile %d\n", THIS_XCORE_TILE);
#endif

    while (1) {
#if DEBUG_LOW_POWER_TASK
        debug_printf("power_control_task() on tile %d entered: %d\n", THIS_XCORE_TILE, state);
#endif
        switch (state) {
        case PWR_CTRL_STATE_LOW_POWER_REQUEST:
            low_power_request();
            state = PWR_CTRL_STATE_LOW_POWER_RESPONSE;
            break;
        case PWR_CTRL_STATE_LOW_POWER_RESPONSE:
            state = (low_power_response()) ?
                    PWR_CTRL_STATE_LOW_POWER_READY :
                    PWR_CTRL_STATE_LOW_POWER_REQUEST;
            break;
        case PWR_CTRL_STATE_LOW_POWER_READY:
            low_power_ready();
            state = PWR_CTRL_STATE_FULL_POWER;
            break;
        case PWR_CTRL_STATE_FULL_POWER:
            full_power();
            state = PWR_CTRL_STATE_LOW_POWER_REQUEST;
            break;
        }
    }
}

void power_control_task_create(unsigned priority, void *args)
{
    xTaskCreate((TaskFunction_t)power_control_task,
                RTOS_STRINGIFY(power_control_task),
                RTOS_THREAD_STACK_SIZE(power_control_task), args,
                priority, &ctx_power_control_task);
}

#if ON_TILE(POWER_CONTROL_TILE_NO)

void power_control_exit_low_power(void)
{
    xTaskNotify(ctx_power_control_task, TASK_NOTIF_MASK_LP_EXIT, eSetBits);
}

power_state_t power_control_state_get(void)
{
    return power_state;
}

#else

void power_control_req_low_power(void)
{
    xTaskNotify(ctx_power_control_task, TASK_NOTIF_MASK_LP_ENTER, eSetBits);
}

void power_control_ind_complete(void)
{
    xTaskNotify(ctx_power_control_task, TASK_NOTIF_MASK_LP_IND_COMPLETE, eSetBits);
}

#endif /* ON_TILE(POWER_CONTROL_TILE_NO) */
