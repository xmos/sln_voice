// Copyright 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
// XMOS Public License: Version 1

/* System headers */
#include <platform.h>

/* App headers */
#include "platform/driver_instances.h"
#include "hostactive.h"

#define USER_ACTIVE_LED_PIN (4)
static rtos_gpio_port_id_t host_active_led_port;

void UserHostActive_LED_Init()
{
    // Inititalise host active LED port
    host_active_led_port = rtos_gpio_port(PORT_GPO);
    rtos_gpio_port_enable(gpio_ctx_t0, host_active_led_port);
    UserHostActive(0); // Turn LED off by default
}

void UserHostActive(int active)
{
    uint32_t port_val = rtos_gpio_port_in(gpio_ctx_t0, host_active_led_port);
    int bit = USER_ACTIVE_LED_PIN;
    uint32_t active_level_bitmap = 0;
    uint32_t mask = ~((unsigned)1 << bit);
    port_val &= mask; // Zero out the relevant bit
    uint32_t state_off = (active_level_bitmap) & mask;
    uint32_t state_on = active_level_bitmap & mask;
    if(active)
    {
        port_val |= state_on;
    }
    else
    {
        port_val |= state_off;
    }
    rtos_gpio_port_out(gpio_ctx_t0, host_active_led_port, port_val);
    return;
}
