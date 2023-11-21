// Copyright 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"

/* Library headers */
#include "fs_support.h"

/* App headers */
#include "platform_conf.h"
#include "platform/driver_instances.h"
#include "usb_support.h"

#include "asrc_utils.h"

extern void i2s_rate_conversion_enable(void);
extern void configure_io_expander(void);

static void flash_start(void)
{
#if ON_TILE(FLASH_TILE_NO)
    uint32_t flash_core_map = ~((1 << appconfUSB_INTERRUPT_CORE) | (1 << appconfUSB_SOF_INTERRUPT_CORE));
    rtos_qspi_flash_start(qspi_flash_ctx, appconfQSPI_FLASH_TASK_PRIORITY);
    rtos_qspi_flash_op_core_affinity_set(qspi_flash_ctx, flash_core_map);
#endif
}

static void i2c_master_start(void)
{
#if ON_TILE(I2C_TILE_NO)
    rtos_i2c_master_start(i2c_master_ctx);
#endif
}

static void enable_level_shifters(void)
{
#if appconfI2S_ENABLED
    int ret = 0;
#if ON_TILE(I2C_TILE_NO)
    configure_io_expander();
    rtos_intertile_tx(intertile_ctx, 0, &ret, sizeof(ret));
#else
    rtos_intertile_rx_len(intertile_ctx, 0, RTOS_OSAL_WAIT_FOREVER);
    rtos_intertile_rx_data(intertile_ctx, &ret, sizeof(ret));
#endif
#endif
}

static void i2s_start(void)
{
#if appconfI2S_ENABLED
#if ON_TILE(I2S_TILE_NO)
    rtos_i2s_start(
            i2s_ctx,
            0, // Not used for I2S slave
            I2S_MODE_I2S,
            2.2 * I2S_TO_USB_ASRC_BLOCK_LENGTH,
            4 * USB_TO_I2S_ASRC_BLOCK_LENGTH * 4 * (appconfI2S_TDM_ENABLED ? 3 : 1), // factor of 4 for the worst case 48 -> 192kHz upsampling.
                                                                                     // An extra factor of 4 to account for jitter during startup.
            appconfI2S_INTERRUPT_CORE);
#endif
#endif
}

static void usb_start(void)
{
#if appconfUSB_ENABLED && ON_TILE(USB_TILE_NO)
    usb_manager_start(appconfUSB_MGR_TASK_PRIORITY);
#endif
}

void platform_start(void)
{
    rtos_intertile_start(intertile_ctx);
    rtos_intertile_start(intertile_usb_audio_ctx);
    rtos_intertile_start(intertile_i2s_audio_ctx);
    flash_start();
    i2c_master_start();
    enable_level_shifters();
    i2s_start();
    usb_start();
}
