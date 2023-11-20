// Copyright 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"

/* App headers */
#include "platform_conf.h"
#include "platform/driver_instances.h"
#include "dac3101.h"
#include "pcal6408a.h"

/* I2C io expander address on XCF3610_Q60A board */
#define IOEXP_I2C_ADDR        PCAL6408A_I2C_ADDR

/* IO expander pinout */
#define XVF_RST_N_PIN   0
#define INT_N_PIN       1
#define DAC_RST_N_PIN   2
#define BOOT_SEL_PIN    3
#define MCLK_OE_PIN     4
#define SPI_OE_PIN      5
#define I2S_OE_PIN      6
#define MUTE_PIN        7

int dac3101_reg_write(uint8_t reg, uint8_t val)
{
    i2c_regop_res_t ret;

    ret = rtos_i2c_master_reg_write(i2c_master_ctx, DAC3101_I2C_DEVICE_ADDR, reg, val);

    if (ret == I2C_REGOP_SUCCESS) {
        return 0;
    } else {
        return -1;
    }
}

void dac3101_codec_reset(void)
{
    /* Set DAC_RST_N to 0 and enable level shifters */
    i2c_regop_res_t ret;
    uint8_t bitmask = (1<<XVF_RST_N_PIN) |
                      (1<<INT_N_PIN)     |
                      (1<<BOOT_SEL_PIN)  |
                      (1<<MCLK_OE_PIN)   |
                      (1<<SPI_OE_PIN)    |
                      (1<<I2S_OE_PIN)    |
                      (1<<MUTE_PIN);
    ret = rtos_i2c_master_reg_write(i2c_master_ctx, IOEXP_I2C_ADDR, PCAL6408A_OUTPUT_PORT, bitmask);
    if (ret != I2C_REGOP_SUCCESS) {
        rtos_printf("Failed to set io expander output port!\n");
    }
    vTaskDelay(pdMS_TO_TICKS(100));

    /* Pin directions */
    bitmask = (1<<XVF_RST_N_PIN) |
              (1<<INT_N_PIN)     |
              (1<<BOOT_SEL_PIN)  |
              (1<<MUTE_PIN);
    ret = rtos_i2c_master_reg_write(i2c_master_ctx, IOEXP_I2C_ADDR, PCAL6408A_CONF, bitmask);
    if (ret != I2C_REGOP_SUCCESS) {
        rtos_printf("Failed to set io expander configuration!\n");
    }
    vTaskDelay(pdMS_TO_TICKS(100));

    /* Enable interrupts */
    bitmask = 0xFF & ~(1<<INT_N_PIN);
    ret = rtos_i2c_master_reg_write(i2c_master_ctx, IOEXP_I2C_ADDR, PCAL6408A_INTERRUPT_MASK, bitmask);
    if (ret != I2C_REGOP_SUCCESS) {
        rtos_printf("Failed to set io expander interrupt mask!\n");
    }
    vTaskDelay(pdMS_TO_TICKS(100));

    /* Reset the dac */
    bitmask = (1<<XVF_RST_N_PIN) |
              (1<<INT_N_PIN)     |
              (1<<DAC_RST_N_PIN) |
              (1<<BOOT_SEL_PIN)  |
              (1<<MCLK_OE_PIN)   |
              (1<<SPI_OE_PIN)    |
              (1<<I2S_OE_PIN)    |
              (1<<MUTE_PIN);
    ret = rtos_i2c_master_reg_write(i2c_master_ctx, IOEXP_I2C_ADDR, PCAL6408A_OUTPUT_PORT, bitmask);
    if (ret != I2C_REGOP_SUCCESS) {
        rtos_printf("Failed to set io expander output!\n");
    }
    vTaskDelay(pdMS_TO_TICKS(100));
}

void dac3101_wait(uint32_t wait_ms)
{
    vTaskDelay(pdMS_TO_TICKS(wait_ms));
}
