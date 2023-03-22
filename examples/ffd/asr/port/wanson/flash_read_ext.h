// Copyright 2021-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef FLASH_READ_EXT_H_
#define FLASH_READ_EXT_H_

#include <stddef.h>
#include <xs1.h>

// NOTE: Wanson ASR engine calls the swmem_load function.
//       However, this is confusing given swmem is not used.
//       We use the macro below as an attempt to mitigate this confusion.
#define flash_read_ext(...) swmem_load(__VA_ARGS__)

/**
 * Load model data.
 *
 * @param[out] dest Pointer to the memory location to copy to
 * @param[in]  src  Pointer to the memory location to copy from
 * @param[in]  size Number of bytes to copy
 */
size_t flash_read_ext(void *dest, const void *src, size_t size);

#endif  // FLASH_READ_EXT_H_
