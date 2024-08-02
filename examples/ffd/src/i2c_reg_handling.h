// Copyright 2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "rtos_i2c_slave.h"
#include "asr.h"

#define WAKEWORD_REG_ADDRESS_START  0x40
#define WAKEWORD_REG_ADDRESS_END    0x49
#define WRITE_REQUEST_MIN_LEN       1

RTOS_I2C_SLAVE_CALLBACK_ATTR
size_t read_device_reg(rtos_i2c_slave_t *ctx,
                              asr_result_t *last_asr_result,
                              uint8_t **data);

RTOS_I2C_SLAVE_CALLBACK_ATTR
void write_device_reg(rtos_i2c_slave_t *ctx,
                              void *app_data,
                              uint8_t *data,
                              size_t len);