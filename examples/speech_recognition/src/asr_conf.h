// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef ASR_CONF_H
#define ASR_CONF_H

#include <stdio.h>
#include <stdlib.h>

#include "flash.h"

#define IS_FLASH(a)                    \
   (((uintptr_t)a >= XS1_SWMEM_BASE) && \
   (((uintptr_t)a <= (XS1_SWMEM_BASE - 1 + XS1_SWMEM_SIZE))))

// Implementation of ASR_READ_EXT which will copy n bytes from an address (src)
// into a caller specified buffer (dest). Read from flash if the src address is 
// in the flash address space, otherwise use memcpy.
inline void *flash_read_ext(void *dest, const void *src, size_t n) {
    if IS_FLASH(src) {
        flash_read_wrapper((unsigned *)dest, (unsigned)src, n);
    } else {
        memcpy(dest, src, n);
    }
    return dest;
}

// #define ASR_MALLOC   malloc
// #define ASR_FREE     free

#define ASR_READ_EXT	flash_read_ext

#endif // ASR_CONF_H