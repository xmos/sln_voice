
// Copyright 2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "i2c_reg_handling.h"

/**
 * @brief Minimum length for a write request.
 */
#define WRITE_REQUEST_MIN_LEN   1

RTOS_I2C_SLAVE_CALLBACK_ATTR
size_t read_device_reg(rtos_i2c_slave_t *ctx,
                              asr_result_t *last_asr_result,
                              uint8_t **data)
{
#if appconfI2C_SLAVE_ENABLED==1
    uint8_t * data_p = *data;
    uint8_t reg_addr = data_p[0];
    uint8_t reg_value = 0xFF;
    if (reg_addr == appconfINTENT_I2C_REG_ADDRESS) {
        reg_value = last_asr_result->id;
    }
    data_p[0] = reg_value;
    rtos_printf("Read from register 0x%02X value 0x%02X\n", reg_addr, reg_value);
#endif
    return 1;
}

RTOS_I2C_SLAVE_CALLBACK_ATTR
void write_device_reg(rtos_i2c_slave_t *ctx,
                              void *app_data,
                              uint8_t *data,
                              size_t len)
{
#if appconfI2C_SLAVE_ENABLED==1
    // If the length is lower than WRITE_REQUEST_MIN_LEN, it is a read request
    if (len > WRITE_REQUEST_MIN_LEN) {
        rtos_printf("Write to register 0x%02X value 0x%02X (len %d)\n", data[0], data[1], len);
    }
#endif
}
