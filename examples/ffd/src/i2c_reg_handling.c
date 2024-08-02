
// Copyright 2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "i2c_reg_handling.h"

#define WAKEWORD_REG_ADDRESS_START  0x40
#define WAKEWORD_REG_ADDRESS_END    0x49
#define WRITE_REQUEST_MIN_LEN       1

RTOS_I2C_SLAVE_CALLBACK_ATTR
size_t read_device_reg(rtos_i2c_slave_t *ctx,
                              asr_result_t *last_asr_result,
                              uint8_t **data)
{
    uint8_t * data_p = *data;
    uint8_t reg_addr = data_p[0];
    uint8_t reg_value = 0;
    if (reg_addr >= WAKEWORD_REG_ADDRESS_START && reg_addr <= WAKEWORD_REG_ADDRESS_END) {
        if (last_asr_result->id == reg_addr - WAKEWORD_REG_ADDRESS_START + 1)
        {
            uint8_t score = (uint8_t) last_asr_result->score&0xF;
            printf("Found wakeword information: ID %d, score %d\n", last_asr_result->id, score);
            reg_value = score;
        } else {
            reg_value = 0;
        }
    } else {
        reg_value = reg_addr - 1;
    }
    data_p[0] = reg_value;
    printf("Read from register 0x%02X value 0x%02X\n", reg_addr, reg_value);
    return 1;
}

RTOS_I2C_SLAVE_CALLBACK_ATTR
void write_device_reg(rtos_i2c_slave_t *ctx,
                              void *app_data,
                              uint8_t *data,
                              size_t len)
{
    // If the length is lower than WRITE_REQUEST_MIN_LEN, it is a read request
    if (len > WRITE_REQUEST_MIN_LEN) {
        printf("Write to register 0x%02X value 0x%02X (len %d)\n", data[0], data[1], len);
    }
}