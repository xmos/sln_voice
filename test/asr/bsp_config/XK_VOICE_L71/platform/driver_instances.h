// Copyright 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef DRIVER_INSTANCES_H_
#define DRIVER_INSTANCES_H_

#include "rtos_intertile.h"
#include "rtos_qspi_flash.h"

/* Tile specifiers */
#define FLASH_TILE_NO      0

/** TILE 0 Clock Blocks */
#define FLASH_CLKBLK        XS1_CLKBLK_1

/* Port definitions */
#define PORT_SQI_CS         PORT_SQI_CS_0
#define PORT_SQI_SCLK       PORT_SQI_SCLK_0
#define PORT_SQI_SIO        PORT_SQI_SIO_0

extern rtos_intertile_t *intertile_ctx;
extern rtos_qspi_flash_t *qspi_flash_ctx;

#endif /* DRIVER_INSTANCES_H_ */
