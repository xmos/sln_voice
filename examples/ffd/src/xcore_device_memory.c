// Copyright 2021-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <xcore/assert.h>

/* App headers */
#include "ff.h"
#include "xcore_device_memory.h"

static FIL model_file;

size_t model_file_init() 
{
    FRESULT result;
    
    result = f_open(&model_file, "model.bin", FA_READ);
    if (result == FR_OK) {
        return f_size(&model_file);
    }
    return 0;
}

size_t model_data_load(void *dest, const void *src, size_t size)
{
    xassert(IS_SWMEM(src));
    
    size_t bytes_read;

    f_lseek(&model_file, (FSIZE_t)src - (FSIZE_t)XS1_SWMEM_BASE);
    f_read(&model_file, dest, size, &bytes_read);
    
    return bytes_read;
}
