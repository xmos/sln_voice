// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* System headers */
#include <xcore/hwtimer.h>
#include <xcore/lock.h>
#include <xcore/thread.h>
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
#include "device_memory.h"
#include "device_memory_impl.h"

void asr_printf(const char * format, ...) {
    va_list args;
    va_start(args, format);
    xcore_utils_vprintf(format, args);
    va_end(args);
}

__attribute__((fptrgroup("devmem_malloc_fptr_grp")))
void * devmem_malloc_local(size_t size) {
    //rtos_printf("devmem_malloc_local size=%d\n", size);
    return pvPortMalloc(size);
}

__attribute__((fptrgroup("devmem_free_fptr_grp")))
void devmem_free_local(void *ptr) {
    vPortFree(ptr);
}

__attribute__((fptrgroup("devmem_read_ext_fptr_grp")))
void devmem_read_ext_local(void *dest, const void *src, size_t n) {
    //rtos_printf("devmem_read_ext_local  dest=0x%x    src=0x%x    size=%d\n", dest, src, n);
    if (IS_FLASH(src)) {
        //uint32_t s = get_reference_time();
        int retval = -1; 
        while (retval == -1) {
            // Need to subtract off XS1_SWMEM_BASE because qspi flash driver accounts for the offset
            retval = rtos_qspi_flash_fast_read_mode_ll(qspi_flash_ctx, (uint8_t *)dest, (unsigned)(src - XS1_SWMEM_BASE), n, qspi_fast_flash_read_transfer_raw);
        }
        //uint32_t d = get_reference_time() - s;
        //printf("%d, %0.01f (us), %0.04f (M/s)\n", n, d / 100.0f, (n / 1000000.0f ) / (d / 100000000.0f));
    } else {
        memcpy(dest, src, n);
    }    
}

void devmem_init(devmem_manager_t *devmem_ctx) {
    xassert(devmem_ctx);    
    devmem_ctx->malloc = devmem_malloc_local;
    devmem_ctx->free = devmem_free_local;
    devmem_ctx->read_ext = devmem_read_ext_local;
    devmem_ctx->read_ext_async = NULL;  // not supported in this application
    devmem_ctx->read_ext_wait = NULL;   // not supported in this application
}

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
