// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef ASR_CONFIG_H
#define ASR_CONFIG_H

#include <stdio.h>
#include <stdlib.h>

#include "flash.h"

// Implementation of ASR_READ_EXT which will read n bytes from an 
// address (src) in flash into a caller specified buffer (dest).  
inline void *flash_read_ext(void *dest, const void * src, size_t n) {
    // NOTE: flash_read_wrapper expects size to be in words, not bytes
    flash_read_wrapper((unsigned) src, (unsigned *)dest, n>>2);
    return dest;
}

// #define ASR_MALLOC   malloc
// #define ASR_FREE     free

#define ASR_READ_EXT	flash_read_ext


#endif // ASR_CONFIG_H