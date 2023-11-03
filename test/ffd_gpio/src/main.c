// Copyright 2021-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "queue.h"

/* Library headers */
#include "rtos_gpio.h"
#include "rtos_i2c_master.h"
#include "rtos_i2c_slave.h"
#include "rtos_intertile.h"
#include "rtos_uart_tx.h"
#include "rtos_uart_rx.h"

/* App headers */
#include "app_conf.h"
#include "board_init.h"
#include "rtos_test/rtos_test_utils.h"

/* App header to unit test */
#include "intent_handler/intent_handler.h"

#define WAKEUP_LOW  (appconfINTENT_WAKEUP_EDGE_TYPE)
#define WAKEUP_HIGH (appconfINTENT_WAKEUP_EDGE_TYPE == 0)

static rtos_intertile_t intertile_ctx_s;
static rtos_i2c_master_t i2c_master_ctx_s;
static rtos_i2c_slave_t i2c_slave_ctx_s;
static rtos_gpio_t gpio_ctx_t0_s;
static rtos_gpio_t gpio_ctx_t1_s;
static rtos_uart_tx_t rtos_uart_tx_ctx_s;
static rtos_uart_rx_t rtos_uart_rx_ctx_s;

rtos_intertile_t *intertile_ctx = &intertile_ctx_s;
rtos_i2c_master_t *i2c_master_ctx = &i2c_master_ctx_s;
rtos_i2c_slave_t *i2c_slave_ctx = &i2c_slave_ctx_s;
rtos_gpio_t *gpio_ctx_t0 = &gpio_ctx_t0_s;
rtos_gpio_t *gpio_ctx_t1 = &gpio_ctx_t1_s;
rtos_uart_tx_t *rtos_uart_tx_ctx = &rtos_uart_tx_ctx_s;
rtos_uart_rx_t *rtos_uart_rx_ctx = &rtos_uart_rx_ctx_s;

chanend_t other_tile_c;

#define kernel_printf( FMT, ... )    module_printf("KERNEL", FMT, ##__VA_ARGS__)

void vApplicationMallocFailedHook( void )
{
    kernel_printf("Malloc Failed!");
    configASSERT(0);
}

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char* pcTaskName )
{
    kernel_printf("Stack Overflow! %s", pcTaskName);
    configASSERT(0);
}

void vWD(void *arg)
{
    vTaskDelay(10);
    test_printf("Host Timeout");
    test_printf("FAIL");
    _Exit(0);
}

#if ON_TILE(1)
RTOS_GPIO_ISR_CALLBACK_ATTR
static void gpio_callback(rtos_gpio_t *ctx, void *app_data, rtos_gpio_port_id_t port_id, uint32_t value)
{
    TaskHandle_t task = app_data;
    BaseType_t yield_required = pdFALSE;

    xTaskNotifyFromISR(task, value, eSetValueWithOverwrite, &yield_required);

    portYIELD_FROM_ISR(yield_required);
}
#endif

void vApplicationDaemonTaskStartup(void *arg)
{
    sync(other_tile_c);

#if ON_TILE(0)
    // Setup and wait for "host" to be in initial state
    QueueHandle_t q_intent = xQueueCreate(1, sizeof(int32_t));
    intent_handler_create(1, q_intent);

    vTaskDelay(1); // Provide some time for proc_keyword_res() to set up
    sync(other_tile_c);

    // Send intent
    int32_t test_val = 50;
    test_printf("Send dummy intent");
    xQueueSend(q_intent, &test_val, (TickType_t)0);
    test_printf("Sent dummy intent");

    test_printf("PASS GPIO");
    sync(other_tile_c);

    while(1) {;}
#endif

#if ON_TILE(1)
    uint32_t value = 0;

    // Setup watchdog-like task to fail after timeout if transitions are not seen
    xTaskCreate((TaskFunction_t) vWD,
                "vWD",
                RTOS_THREAD_STACK_SIZE(vWD),
                NULL,
                appconfSTARTUP_TASK_PRIORITY-1,
                NULL);

    // Setup initial "host" state
    const rtos_gpio_port_id_t p_host_status = rtos_gpio_port(GPIO_HOST_TO_BOARD_HOST_STATUS_PORT);
    const rtos_gpio_port_id_t p_in = rtos_gpio_port(GPIO_BOARD_TO_HOST_WAKEUP_PORT);

    rtos_gpio_port_enable(gpio_ctx_t1, p_host_status);
    rtos_gpio_port_enable(gpio_ctx_t1, p_in);

    rtos_gpio_port_out(gpio_ctx_t1, p_host_status, 0);

    rtos_gpio_isr_callback_set(gpio_ctx_t1, p_in, gpio_callback, xTaskGetCurrentTaskHandle());
    rtos_gpio_interrupt_enable(gpio_ctx_t1, p_in);

    sync(other_tile_c);
    
    // Wait for wakeup request
    xTaskNotifyWait(
            0x00000000UL,          /* Don't clear notification bits on entry */
            0xFFFFFFFFUL,          /* Reset full notification value on exit */
            &value,                /* Pass out notification value into value */
            pdMS_TO_TICKS(1000) ); /* Wait time or fail test */

    test_printf("Host wake up received rising edge");

    xTaskNotifyWait(
            0x00000000UL,          /* Don't clear notification bits on entry */
            0xFFFFFFFFUL,          /* Reset full notification value on exit */
            &value,                /* Pass out notification value into value */
            pdMS_TO_TICKS(1000) ); /* Wait time or fail test */

    test_printf("Host saw falling edge");
    
    // Update host status
    rtos_gpio_port_out(gpio_ctx_t1, p_host_status, 1);
    sync(other_tile_c);

    test_printf("PASS GPIO");

    _Exit(0);
#endif

    chanend_free(other_tile_c);
    vTaskDelete(NULL);
}

#if ON_TILE(0)
void main_tile0(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
{
    (void) c0;
    board_tile0_init(c1,
                     intertile_ctx,
                     i2c_master_ctx,
                     gpio_ctx_t0,
                     rtos_uart_tx_ctx);
    (void) c2;
    (void) c3;

    other_tile_c = c1;

    xTaskCreate((TaskFunction_t) vApplicationDaemonTaskStartup,
                "vApplicationDaemonTaskStartup",
                RTOS_THREAD_STACK_SIZE(vApplicationDaemonTaskStartup),
                NULL,
                appconfSTARTUP_TASK_PRIORITY,
                NULL);

    kernel_printf("Start scheduler");
    vTaskStartScheduler();

    return;
}
#endif /* ON_TILE(0) */

#if ON_TILE(1)
void main_tile1(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
{
    board_tile1_init(c0,
                     intertile_ctx,
                     i2c_slave_ctx,
                     gpio_ctx_t1,
                     rtos_uart_rx_ctx);
    (void) c1;
    (void) c2;
    (void) c3;

    other_tile_c = c0;

    xTaskCreate((TaskFunction_t) vApplicationDaemonTaskStartup,
                "vApplicationDaemonTaskStartup",
                RTOS_THREAD_STACK_SIZE(vApplicationDaemonTaskStartup),
                NULL,
                appconfSTARTUP_TASK_PRIORITY,
                NULL);

    kernel_printf("Start scheduler");
    vTaskStartScheduler();

    return;
}
#endif /* ON_TILE(1) */
