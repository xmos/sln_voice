// Copyright 2021-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <stdint.h>
#include <xs1.h>
#include <xcore/assert.h>

/* Library headers */
#include "rtos_qspi_flash.h"

/* App headers */
#include "platform/driver_instances.h"
#include "xcore_device_memory.h"

/* The offset in flash where the model(s) reside. */
#ifndef QSPI_FLASH_MODEL_START_ADDRESS
#define QSPI_FLASH_MODEL_START_ADDRESS    0x200000
#endif

#define QSPI_FLASH_READ_MIN_SIZE 2

size_t model_data_load(void *dest, const void *src, size_t size)
{
    unsigned offset = (unsigned)src - XS1_SWMEM_BASE +
        QSPI_FLASH_MODEL_START_ADDRESS;

    xassert(IS_SWMEM(src));

    rtos_qspi_flash_lock(qspi_flash_ctx);

    if (size == 1) {
        uint8_t temp_dest[QSPI_FLASH_READ_MIN_SIZE];
        rtos_qspi_flash_read(qspi_flash_ctx,
                             temp_dest,
                             offset,
                             sizeof(temp_dest));
        *(uint8_t *)dest=temp_dest[0];
    } else {
        rtos_qspi_flash_read(qspi_flash_ctx,
                             (uint8_t *)dest,
                             offset,
                             size);
    }

    rtos_qspi_flash_unlock(qspi_flash_ctx);
    return size;
}
