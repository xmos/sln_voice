// Copyright 2021-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef BOARD_INIT_H_
#define BOARD_INIT_H_

#include "rtos_intertile.h"
#include "rtos_gpio.h"
#include "rtos_i2c_master.h"
#include "rtos_i2c_slave.h"
#include "rtos_uart_tx.h"
#include "rtos_uart_rx.h"

void board_tile0_init(
        chanend_t tile1,
        rtos_intertile_t *intertile_ctx,
        rtos_i2c_master_t *i2c_master_ctx,
        rtos_gpio_t *gpio_ctx,
        rtos_uart_tx_t *rtos_uart_tx_ctx
    );

void board_tile1_init(
        chanend_t tile0,
        rtos_intertile_t *intertile_ctx,
        rtos_i2c_slave_t *i2c_slave_ctx,
        rtos_gpio_t *gpio_ctx,
        rtos_uart_rx_t *rtos_uart_rx_ctx
    );

#endif /* BOARD_INIT_H_ */
