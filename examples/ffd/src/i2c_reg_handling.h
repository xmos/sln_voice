// Copyright 2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "rtos_i2c_slave.h"
#include "app_conf.h"
#include "asr.h"

/**
 * Callback for reading data from a device register over I2C.
 * Only one byte of data is read from the register.
 *
 * @param ctx Pointer to the I2C slave context.
 * @param last_asr_result Pointer to the last Automatic Speech Recognition (ASR) result.
 * @param data Pointer to a pointer to the the data received from the master device.
 *
 * @return The size of the data read.
 */
RTOS_I2C_SLAVE_CALLBACK_ATTR
size_t read_device_reg(rtos_i2c_slave_t *ctx,
                       asr_result_t *last_asr_result,
                       uint8_t **data);

/**
 * Callback for writing data to a device register over I2C.
 * Only one byte of data is written to the register.
 *
 * @param ctx Pointer to the I2C slave context.
 * @param app_data Pointer to application-specific data. Not used.
 * @param data Pointer pointer to the the data received from the master device.
 * @param len The length of the data to be written.
 */
RTOS_I2C_SLAVE_CALLBACK_ATTR
void write_device_reg(rtos_i2c_slave_t *ctx,
                      void *app_data,
                      uint8_t *data,
                      size_t len);
