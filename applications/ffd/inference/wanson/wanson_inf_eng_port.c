// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

/* STD headers */
#include <platform.h>
#include <xs1.h>
#include <xcore/hwtimer.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"
#include "queue.h"

/* App headers */
#include "app_conf.h"
#include "platform/driver_instances.h"
#include "inference_engine.h"
#include "wanson_inf_eng.h"
#include "ssd1306_rtos_support.h"

static QueueHandle_t q_intent = 0;

__attribute__((weak))
void wanson_engine_proc_keyword_result(const char **text, int id)
{
    rtos_printf("%s 0x%x\n", (char*)*text, id);
    if(q_intent != 0) {
        if(xQueueSend(q_intent, (void *)&id, (TickType_t)0) != pdPASS) {
            rtos_printf("Lost intent.  Queue was full.\n");
        }
    }

#if appconfSSD1306_DISPLAY_ENABLED
    // some temporary fixes to the strings returned
    switch (id) {
        case 200:
            // fix capital "On"
            ssd1306_display_ascii_to_bitmap("Switch on the TV\0");
            break;
        case 420:
            // fix lower case "speed"
            // fix word wrapping
            ssd1306_display_ascii_to_bitmap("Speed up the   fan\0");
            break;
        case 430:
            // fix lower case "slow"
            ssd1306_display_ascii_to_bitmap("Slow down the fan\0");
            break;
        case 440:
            // fix lower case "set"
            // fix word wrapping
            ssd1306_display_ascii_to_bitmap("Set higher    temperature\0");
            break;
        case 450:
            // fix lower case "set"
            // fix word wrapping
            ssd1306_display_ascii_to_bitmap("Set lower     temperature\0");
            break;
        default:
            ssd1306_display_ascii_to_bitmap((char *)*text);
    }
#endif
}

#define WAKEUP_LOW  (appconfINTENT_WAKEUP_EDGE_TYPE)
#define WAKEUP_HIGH (appconfINTENT_WAKEUP_EDGE_TYPE == 0)

static void proc_keyword_res(void *args) {
    (void) args;
    int32_t id = 0;
    int32_t host_status = 0;

    configASSERT(q_intent != 0);

    const rtos_gpio_port_id_t p_out_wakeup = rtos_gpio_port(XS1_PORT_1D);       /* PORT_SPI_MOSI on XK_VOICE_L71*/
    const rtos_gpio_port_id_t p_in_host_status = rtos_gpio_port(XS1_PORT_1P);   /* PORT_SPI_MISO on XK_VOICE_L71*/

    rtos_gpio_port_enable(gpio_ctx_t0, p_out_wakeup);
    rtos_gpio_port_enable(gpio_ctx_t0, p_in_host_status);

    rtos_gpio_port_out(gpio_ctx_t0, p_out_wakeup, WAKEUP_LOW);

    while(1) {
        xQueueReceive(q_intent, &id, portMAX_DELAY);

        host_status = rtos_gpio_port_in(gpio_ctx_t0, p_in_host_status);

        if (host_status == 0) { /* Host is not awake */
            rtos_gpio_port_out(gpio_ctx_t0, p_out_wakeup, WAKEUP_HIGH);
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
    }
}

int32_t inference_engine_create(uint32_t priority, void *args)
{
    (void) args;

#if appconfINFERENCE_ENABLED
    q_intent = xQueueCreate(appconfINTENT_QUEUE_LEN, sizeof(int32_t));

    xTaskCreate((TaskFunction_t)proc_keyword_res,
                "proc_keyword_res",
                RTOS_THREAD_STACK_SIZE(proc_keyword_res),
                NULL,
                priority,
                NULL);
#if INFERENCE_TILE_NO == AUDIO_PIPELINE_TILE_NO
    wanson_engine_task_create(priority);
#else
    wanson_engine_intertile_task_create(priority);
#endif
#endif
    return 0;
}

int32_t inference_engine_sample_push(int32_t *buf, size_t frames)
{
#if appconfINFERENCE_ENABLED
#if INFERENCE_TILE_NO == AUDIO_PIPELINE_TILE_NO
    wanson_engine_samples_send_local(
            frames,
            buf);
#else
    wanson_engine_samples_send_remote(
            intertile_ctx,
            frames,
            buf);
#endif
#endif
    return 0;
}
