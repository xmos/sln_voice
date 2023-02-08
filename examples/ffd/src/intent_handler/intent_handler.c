// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

/* STD headers */
#include <platform.h>
#include <xs1.h>
#include <xcore/hwtimer.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* App headers */
#include "app_conf.h"
#include "platform/driver_instances.h"
#include "intent_handler/intent_handler.h"
#include "fs_support.h"
#include "ff.h"
#include "audio_response.h"
#include "inference_engine.h"
#include "power/lp_control.h"

#define WAKEUP_LOW  (appconfINTENT_WAKEUP_EDGE_TYPE)
#define WAKEUP_HIGH (appconfINTENT_WAKEUP_EDGE_TYPE == 0)

#if ON_TILE(INFERENCE_TILE_NO)

static void proc_keyword_res(void *args) {
    QueueHandle_t q_intent = (QueueHandle_t) args;
    int32_t id = 0;
    int32_t host_status = 0;

    configASSERT(q_intent != 0);

    const rtos_gpio_port_id_t p_out_wakeup = rtos_gpio_port(GPIO_OUT_HOST_WAKEUP_PORT);
    const rtos_gpio_port_id_t p_in_host_status = rtos_gpio_port(GPIO_IN_HOST_STATUS_PORT);

    rtos_gpio_port_enable(gpio_ctx_t0, p_out_wakeup);
    rtos_gpio_port_enable(gpio_ctx_t0, p_in_host_status);

    rtos_gpio_port_out(gpio_ctx_t0, p_out_wakeup, WAKEUP_LOW);

#if appconfAUDIO_PLAYBACK_ENABLED
    audio_response_init();
#endif

#if appconfLOW_POWER_ENABLED
    lp_slave_user_not_active(lp_ctx, LP_SLAVE_LP_INT_HANDLER);
#endif
    while(1) {
        xQueueReceive(q_intent, &id, portMAX_DELAY);
#if appconfLOW_POWER_ENABLED
        lp_slave_user_active(lp_ctx, LP_SLAVE_LP_INT_HANDLER);
#endif

        host_status = rtos_gpio_port_in(gpio_ctx_t0, p_in_host_status);

        if (host_status == 0) { /* Host is not awake */
            rtos_gpio_port_out(gpio_ctx_t0, p_out_wakeup, WAKEUP_HIGH);
            rtos_printf("Delay for host wake up\n");
            vTaskDelay(pdMS_TO_TICKS(appconfINTENT_TRANSPORT_DELAY_MS));
            rtos_gpio_port_out(gpio_ctx_t0, p_out_wakeup, WAKEUP_LOW);
        }
#if appconfINFERENCE_I2C_OUTPUT_ENABLED
        i2c_res_t ret;
        uint32_t buf = id;
        size_t sent = 0;

        ret = rtos_i2c_master_write(
            i2c_master_ctx,
            appconfINFERENCE_I2C_OUTPUT_DEVICE_ADDR,
            (uint8_t*)&buf,
            sizeof(uint32_t),
            &sent,
            1
        );

        if (ret != I2C_ACK) {
            rtos_printf("I2C inference output was not acknowledged\n\tSent %d bytes\n", sent);
        }
#endif
#if appconfINFERENCE_UART_OUTPUT_ENABLED && (UART_TILE_NO == INFERENCE_TILE_NO)
        uint32_t buf_uart = id;
        rtos_uart_tx_write(uart_tx_ctx, (uint8_t*)&buf_uart, sizeof(uint32_t));
#endif
#if appconfAUDIO_PLAYBACK_ENABLED
        audio_response_play(id);
#endif
#if appconfLOW_POWER_ENABLED
        lp_slave_user_not_active(lp_ctx, LP_SLAVE_LP_INT_HANDLER);
#endif
    }
}

int32_t intent_handler_create(uint32_t priority, void *args)
{
    xTaskCreate((TaskFunction_t)proc_keyword_res,
                "proc_keyword_res",
                RTOS_THREAD_STACK_SIZE(proc_keyword_res),
                args,
                priority,
                NULL);

    return 0;
}

#endif /* ON_TILE(INFERENCE_TILE_NO) */
