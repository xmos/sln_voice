// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* System headers */
#include <xcore/assert.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"

/* FatFS headers */
#include "ff.h"            /* Obtains integer types */
#include "diskio.h"        /* Declarations of disk functions */

#ifndef QSPI_FLASH_SECTOR_SIZE
#define QSPI_FLASH_SECTOR_SIZE 4096
#endif

extern DSTATUS drive_status[FF_VOLUMES];

/* Library headers */
#include "rtos_printf.h"
#include "rtos_qspi_flash.h"

/* App headers */
#include "app_conf.h"
#include "platform/driver_instances.h"

// NOTE: Mixed ll (low-level) and non-ll reads are not currently supported by the QSPI flash driver. 
// We are providing a "strong" implementation of the FatFS disk_read function in order to utilize 
// the rtos_qspi_flash_fast_read_mode_ll function for reading.
DRESULT disk_read(BYTE pdrv,  /* Physical drive number to identify the drive */
                  BYTE *buff, /* Data buffer to store read data */
                  LBA_t sector, /* Start sector in LBA */
                  UINT count    /* Number of sectors to read */
) {
    extern rtos_qspi_flash_t *ff_qspi_flash_ctx;
    DRESULT res;
    switch (pdrv) {
#if FF_VOLUMES >= 1
    case 0:
        if ((drive_status[pdrv] & ~STA_PROTECT) == 0) {
            int retval = -1; 
            while (retval == -1) {
                retval = rtos_qspi_flash_fast_read_mode_ll(ff_qspi_flash_ctx, buff, QSPI_FLASH_FILESYSTEM_START_ADDRESS + (sector * QSPI_FLASH_SECTOR_SIZE), count * QSPI_FLASH_SECTOR_SIZE, qspi_fast_flash_read_transfer_nibble_swap);
            }
          res = RES_OK;
        } else if (drive_status[pdrv] & STA_NOINIT) {
          res = RES_NOTRDY;
        } else {
          res = RES_ERROR;
        }
        break;
#endif
    default:
        res = RES_PARERR;
        break;
    }
    return res;
}
