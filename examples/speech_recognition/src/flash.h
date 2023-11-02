// Copyright 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef FLASH_H_
#define FLASH_H_

void flash_setup();
void flash_teardown();
void flash_read_wrapper(unsigned *dest, unsigned src, size_t size);

#endif /* FLASH_H_ */