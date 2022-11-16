// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

/* System headers */
#include <platform.h>
#include <xs1.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"

/* Library headers */
#include "rtos_macros.h"

/* App headers */
#include "app_conf.h"
#include "platform/driver_instances.h"
#include "gpio_ctrl/leds.h"
#include "power/power_state.h"
#include "power/power_control.h"

#define TASK_NOTIF_MASK_LP_ENTER         1  // Used by tile: POWER_CONTROL_TILE_NO
#define TASK_NOTIF_MASK_LP_EXIT          2  // Used by tile: POWER_CONTROL_TILE_NO
#define TASK_NOTIF_MASK_LP_IND_COMPLETE  4  // Used by tile: !POWER_CONTROL_TILE_NO

static TaskHandle_t ctx_power_control_task = NULL;

#if ON_TILE(POWER_CONTROL_TILE_NO)

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

#endif /* ON_TILE(1) */

static void power_control_task(void *arg)
{
    const uint32_t bits_to_clear_on_entry = 0x00000000UL;
    const uint32_t bits_to_clear_on_exit = 0xFFFFFFFFUL;
    power_state_t requested_power_state = POWER_STATE_FULL;
    uint32_t notif_value;

    while (1) {
#if ON_TILE(POWER_CONTROL_TILE_NO)
        xTaskNotifyWait(bits_to_clear_on_entry,
                        bits_to_clear_on_exit,
                        &notif_value,
                        portMAX_DELAY);

        if (notif_value & TASK_NOTIF_MASK_LP_EXIT) {
            // Ignore the event if already exited low power mode.
            if (requested_power_state == POWER_STATE_FULL)
                continue;

            requested_power_state = POWER_STATE_FULL;
            debug_printf("Exiting low power...\n");
            low_power_clocks_disable();
            driver_control_unlock();
            debug_printf("Exited low power.\n");
        } else if (notif_value & TASK_NOTIF_MASK_LP_ENTER) {
            // Ignore the event if already enterred low power mode.
            if (requested_power_state == POWER_STATE_LOW)
                continue;

            requested_power_state = POWER_STATE_LOW;
        }

        // Send the requested power state to the other tile.
        rtos_intertile_tx(intertile_ctx,
                          appconfPOWER_CONTROL_PORT,
                          &requested_power_state,
                          sizeof(requested_power_state));

        // Wait for a response form other tile (the value is not used/important).
        power_state_t power_state_response;
        size_t len_rx = rtos_intertile_rx_len(intertile_ctx, appconfPOWER_CONTROL_PORT, RTOS_OSAL_WAIT_FOREVER);
        configASSERT(len_rx == sizeof(power_state_response));
        rtos_intertile_rx_data(intertile_ctx, &power_state_response, sizeof(power_state_response));

        if (requested_power_state == POWER_STATE_LOW) {
            debug_printf("Entering low power...\n");
            driver_control_lock();
            low_power_clocks_enable();
            debug_printf("Entered low power.\n");
        }
#else
        // Wait for other tile to send the requested power state.
        size_t len_rx = rtos_intertile_rx_len(intertile_ctx, appconfPOWER_CONTROL_PORT, RTOS_OSAL_WAIT_FOREVER);
        configASSERT(len_rx == sizeof(requested_power_state));
        rtos_intertile_rx_data(intertile_ctx, &requested_power_state, sizeof(requested_power_state));

        if (requested_power_state == POWER_STATE_FULL) {
            driver_control_unlock();
            led_indicate_awake();

            /* Wait for a notification, signaling that the LED indication has
             * been applied. */
            xTaskNotifyWait(bits_to_clear_on_entry,
                            bits_to_clear_on_exit,
                            &notif_value,
                            portMAX_DELAY);
        } else {
            led_indicate_asleep();

            /* Wait for a notification, signaling that the LED indication has
             * been applied and that the tile is ready to be set to low power
             * mode by the other tile. */
            xTaskNotifyWait(bits_to_clear_on_entry,
                            bits_to_clear_on_exit,
                            &notif_value,
                            portMAX_DELAY);
            driver_control_lock();
        }

        rtos_intertile_tx(intertile_ctx,
                          appconfPOWER_CONTROL_PORT,
                          &requested_power_state,
                          sizeof(requested_power_state));
#endif
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

void power_control_enter_low_power(void)
{
    xTaskNotify(ctx_power_control_task, TASK_NOTIF_MASK_LP_ENTER, eSetBits);
}

void power_control_exit_low_power(void)
{
    xTaskNotify(ctx_power_control_task, TASK_NOTIF_MASK_LP_EXIT, eSetBits);
}

#else

void power_control_req_complete(void)
{
    xTaskNotify(ctx_power_control_task, TASK_NOTIF_MASK_LP_IND_COMPLETE, eSetBits);
}

#endif
