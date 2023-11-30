// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#pragma once

#include "rtos_gpio.h"
#include "rtos_i2c_master.h"
#include "rtos_i2c_slave.h"
#include "rtos_intertile.h"
#include "rtos_uart_tx.h"
#include "rtos_uart_rx.h"

extern rtos_intertile_t *intertile_ctx;
extern rtos_i2c_master_t *i2c_master_ctx;
extern rtos_i2c_slave_t *i2c_slave_ctx;
extern rtos_gpio_t *gpio_ctx_t0;
extern rtos_gpio_t *gpio_ctx_t1;
extern rtos_uart_tx_t *rtos_uart_tx_ctx;
extern rtos_uart_rx_t *rtos_uart_rx_ctx;