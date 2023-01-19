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
#include "ssd1306_rtos_support.h"
#include "power/power_state.h"
#include "power/power_status.h"

static void proc_power_status(void *args) {

    while (1) {
        power_state_t power_state = 0;
        size_t len_rx = 0;

        len_rx = rtos_intertile_rx_len(intertile_ctx, appconfPOWER_STATE_PORT, RTOS_OSAL_WAIT_FOREVER);
        configASSERT(len_rx == sizeof(power_state));

        rtos_intertile_rx_data(intertile_ctx, &power_state, sizeof(power_state));
        if (power_state == POWER_STATE_LOW) {
#if appconfSSD1306_DISPLAY_ENABLED
            ssd1306_display_ascii_to_bitmap("Low power\0");
#endif
            rtos_printf("POWER_MODE: %d, Low\n", power_state);

        } else if (power_state == POWER_STATE_FULL) {
#if appconfSSD1306_DISPLAY_ENABLED
            ssd1306_display_ascii_to_bitmap("Full power\0");
#endif
            rtos_printf("POWER_MODE: %d, Full\n", power_state);
        }
    }
}

int32_t power_status_create(uint32_t priority, void *args)
{
    xTaskCreate((TaskFunction_t)proc_power_status,
                "proc_power_status",
                RTOS_THREAD_STACK_SIZE(proc_power_status),
                args,
                priority,
                NULL);

    return 0;
}
