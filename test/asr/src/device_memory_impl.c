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

/* Library headers */
#include "rtos_printf.h"

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
        // Need to subtract off XS1_SWMEM_BASE because qspi flash driver accounts for the offset
        //uint32_t s = get_reference_time();
        rtos_qspi_flash_fast_read_mode_ll(qspi_flash_ctx, (uint8_t *)dest, (unsigned)(src - XS1_SWMEM_BASE), n, qspi_fast_flash_read_transfer_raw);
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
