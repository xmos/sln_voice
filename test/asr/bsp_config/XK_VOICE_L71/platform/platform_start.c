// Copyright 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"

/* App headers */
#include "platform_conf.h"
#include "platform/driver_instances.h"

static void flash_start(void)
{
#if ON_TILE(FLASH_TILE_NO)
    rtos_qspi_flash_start(qspi_flash_ctx, appconfQSPI_FLASH_TASK_PRIORITY);
#endif
}

void platform_start(void)
{
    rtos_intertile_start(intertile_ctx);
    flash_start();
}
