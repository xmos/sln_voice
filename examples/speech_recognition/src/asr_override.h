// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef ASR_OVERRIDE_H
#define ASR_OVERRIDE_H

void asr_read_ext(void *dest, const void *src, size_t n);
int asr_read_ext_async(void *dest, const void *src, size_t n);
void asr_read_ext_wait(int handle);

#endif // ASR_OVERRIDE_H