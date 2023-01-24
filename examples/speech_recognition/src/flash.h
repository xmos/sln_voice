// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef FLASH_H_
#define FLASH_H_

#include <xcore/swmem_fill.h>

void flash_setup();
void flash_teardown();
void flash_read_wrapper(unsigned src, unsigned *dest, size_t num_words);

swmem_fill_t swmem_setup();
void swmem_teardown(swmem_fill_t fill_handle);

#endif /* FLASH_H_ */