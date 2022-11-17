// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* STD headers */
#include <platform.h>
#include <xs1.h>
#include <xcore/hwtimer.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"

/* App headers */
#include "app_conf.h"
#include "gpio_ctrl/leds.h"
#include "platform/driver_instances.h"

#define LED_BLINK_DELAY (500 / portTICK_PERIOD_MS)

rtos_gpio_port_id_t gpo_port = 0;

#if XK_VOICE_L71
#define gpo_setup()     {                                       \
    gpo_port = rtos_gpio_port(PORT_GPO);                        \
    rtos_gpio_port_enable(gpio_ctx_t0, gpo_port);               \
}
#define green_led_on()  {                                       \
    uint32_t val = rtos_gpio_port_in(gpio_ctx_t0, gpo_port);    \
    rtos_gpio_port_out(gpio_ctx_t0, gpo_port, val &= ~(1<<5));  \
}
#define green_led_off()  {                                      \
    uint32_t val = rtos_gpio_port_in(gpio_ctx_t0, gpo_port);    \
    rtos_gpio_port_out(gpio_ctx_t0, gpo_port, val |= (1<<5));   \
}
#define red_led_on()  {                                         \
    uint32_t val = rtos_gpio_port_in(gpio_ctx_t0, gpo_port);    \
    rtos_gpio_port_out(gpio_ctx_t0, gpo_port, val &= ~(1<<4));  \
}
#define red_led_off()  {                                        \
    uint32_t val = rtos_gpio_port_in(gpio_ctx_t0, gpo_port);    \
    rtos_gpio_port_out(gpio_ctx_t0, gpo_port, val |= (1<<4));   \
}

#elif XCOREAI_EXPLORER
/* LED 0 is "green"
 * LED 1 is "red" */
#define gpo_setup()     {                                      \
    gpo_port = rtos_gpio_port(PORT_LEDS);                      \
    rtos_gpio_port_enable(gpio_ctx_t0, gpo_port);              \
}
#define green_led_on()  {                                      \
    uint32_t val = rtos_gpio_port_in(gpio_ctx_t0, gpo_port);   \
    rtos_gpio_port_out(gpio_ctx_t0, gpo_port, val |= (1<<0));  \
}
#define green_led_off()  {                                     \
    uint32_t val = rtos_gpio_port_in(gpio_ctx_t0, gpo_port);   \
    rtos_gpio_port_out(gpio_ctx_t0, gpo_port, val &= ~(1<<0)); \
}
#define red_led_on()  {                                        \
    uint32_t val = rtos_gpio_port_in(gpio_ctx_t0, gpo_port);   \
    rtos_gpio_port_out(gpio_ctx_t0, gpo_port, val |= (1<<1));  \
}
#define red_led_off()  {                                       \
    uint32_t val = rtos_gpio_port_in(gpio_ctx_t0, gpo_port);   \
    rtos_gpio_port_out(gpio_ctx_t0, gpo_port, val &= ~(1<<1)); \
}
#endif

void led_heartbeat_task(void *args)
{
    gpo_setup();

    while (1) {
        green_led_on();
        vTaskDelay(LED_BLINK_DELAY);
        green_led_off();
        vTaskDelay(LED_BLINK_DELAY);
    }

}

void led_heartbeat_create(unsigned priority, void *args)
{
    xTaskCreate((TaskFunction_t) led_heartbeat_task,
                "inf_led_heartbeat",
                RTOS_THREAD_STACK_SIZE(led_heartbeat_task),
                args,
                priority,
                NULL);
}
