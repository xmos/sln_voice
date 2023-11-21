// Copyright 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>

/* App headers */
#include "platform_conf.h"
#include "platform/driver_instances.h"
#include "platform/platform_init.h"

static void flash_init(void)
{
#if ON_TILE(FLASH_TILE_NO)
    rtos_qspi_flash_fast_read_init(
            qspi_flash_ctx,
            FLASH_CLKBLK,
            PORT_SQI_CS,
            PORT_SQI_SCLK,
            PORT_SQI_SIO,
            NULL,
            qspi_fast_flash_read_transfer_nibble_swap,
            3,
            QSPI_FLASH_CALIBRATION_ADDRESS);
#endif
}


void platform_init(chanend_t other_tile_c)
{
    rtos_intertile_init(intertile_ctx, other_tile_c);
    flash_init();
}
