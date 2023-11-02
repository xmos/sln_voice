// Copyright 2021-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <platform.h>
#include <xs1.h>
#include <stdarg.h>
#include <xcore/hwtimer.h>

#include "app_conf.h"
#include "board_init.h"

void board_tile0_init(
        chanend_t tile1,
        rtos_intertile_t *intertile_ctx,
        rtos_i2c_master_t *i2c_master_ctx,
        rtos_gpio_t *gpio_ctx,
        rtos_uart_tx_t *rtos_uart_tx_ctx
    )
{
    rtos_intertile_init(intertile_ctx, tile1);

    rtos_gpio_init(
            gpio_ctx);

    rtos_i2c_master_init(
            i2c_master_ctx,
            XS1_PORT_1E, 0, 0,
            XS1_PORT_1P, 0, 0,
            0,
            10);
}

void board_tile1_init(
        chanend_t tile0,
        rtos_intertile_t *intertile_ctx,
        rtos_i2c_slave_t *i2c_slave_ctx,
        rtos_gpio_t *gpio_ctx,
        rtos_uart_rx_t *rtos_uart_rx_ctx
    )
{
    rtos_intertile_init(intertile_ctx, tile0);

    rtos_gpio_init(
            gpio_ctx);

    rtos_i2c_slave_init(
            i2c_slave_ctx,
            I2C_SLAVE_CORE_MASK,
            XS1_PORT_1E,
            XS1_PORT_1P,
            I2C_SLAVE_ADDR);
}
