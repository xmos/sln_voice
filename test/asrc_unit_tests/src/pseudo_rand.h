// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#pragma once

#include <stdint.h>

int pseudo_rand(int* state);


int8_t   pseudo_rand_int8(unsigned *r);
uint8_t  pseudo_rand_uint8(unsigned *r);
int16_t  pseudo_rand_int16(unsigned *r);
uint16_t pseudo_rand_uint16(unsigned *r);
int32_t  pseudo_rand_int32(unsigned *r);
uint32_t pseudo_rand_uint32(unsigned *r);
int64_t  pseudo_rand_int64(unsigned *r);
uint64_t pseudo_rand_uint64(unsigned *r);

int32_t  pseudo_rand_int(unsigned *r, int32_t min, int32_t max);
uint32_t pseudo_rand_uint(unsigned *r, uint32_t min, uint32_t max);

void pseudo_rand_bytes(unsigned *r, char* buffer, unsigned size);
