// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include "xcore_device_memory.h"

#include <xcore/assert.h>

#include"platform/driver_instances.h"

#define SWMEM_OFFSET 0x100000

size_t model_data_load(void *dest, const void *src, size_t size)
{
    xassert(IS_SWMEM(src));

    rtos_qspi_flash_lock(qspi_flash_ctx);
    if(size == 1)
    {
        uint8_t temp_dest[2];
        rtos_qspi_flash_read(qspi_flash_ctx, temp_dest,
                                (unsigned)(src + SWMEM_OFFSET - XS1_SWMEM_BASE), 2);
        *(uint8_t *)dest=temp_dest[0];
    }
    else{
    rtos_qspi_flash_read(qspi_flash_ctx, (uint8_t *)dest,
                            (unsigned)(src + SWMEM_OFFSET - XS1_SWMEM_BASE), size);
    }

    rtos_qspi_flash_unlock(qspi_flash_ctx);
    return size;
}
