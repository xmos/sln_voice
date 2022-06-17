// Copyright 2021-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef XCORE_DEVICE_MEMORY_H_
#define XCORE_DEVICE_MEMORY_H_

#include <stddef.h>
#include <xs1.h>

#define IS_SWMEM(a)                    \
  (((uintptr_t)a >= XS1_SWMEM_BASE) && \
   (((uintptr_t)a <= (XS1_SWMEM_BASE - 1 + XS1_SWMEM_SIZE))))

// NOTE: Wanson ASR engine calls the swmem_load function.  
//       However, this is confusing given swmem is not used.  
//       We use the macro below as an attempt to mitigate this confusion.
#define model_data_load(...) swmem_load(__VA_ARGS__) 

/**
 * Load model data from flash.
 *
 * @param[out] dest Pointer to the memory location to copy to
 * @param[in]  src  Pointer to the memory location to copy from
 * @param[in]  size Number of bytes to copy
 */
size_t model_data_load(void *dest, const void *src, size_t size);

#endif  // XCORE_DEVICE_MEMORY_H_
