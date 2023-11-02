// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
// XMOS Public License: Version 1

#include "pseudo_rand.h"

#include <assert.h>

int pseudo_rand(int* state)
{
  const int a = 1664525;
  const int c = 1013904223;
  *state = (int)((long long)a * (*state) + c);
  return *state;
}


int8_t  pseudo_rand_int8(unsigned *r){
    pseudo_rand((int*)r);
    return (int8_t)*r;
}

uint8_t pseudo_rand_uint8(unsigned *r){
    pseudo_rand((int*)r);
    return (uint8_t)*r;
}

int16_t  pseudo_rand_int16(unsigned *r){
    pseudo_rand((int*)r);
    return (int16_t)*r;
}

uint16_t pseudo_rand_uint16(unsigned *r){
    pseudo_rand((int*)r);
    return (uint16_t)*r;
}

int32_t  pseudo_rand_int32(unsigned *r){
    pseudo_rand((int*)r);
    return (int32_t)*r;
}

uint32_t pseudo_rand_uint32(unsigned *r){
    pseudo_rand((int*)r);
    return (uint32_t)*r;
}

int64_t  pseudo_rand_int64(unsigned *r){
    pseudo_rand((int*)r);
    int64_t a = (int64_t)*r;
    pseudo_rand((int*)r);
    int64_t b = (int64_t)*r;
    return (int64_t)(a + (b<<32));
}

uint64_t pseudo_rand_uint64(unsigned *r){
    pseudo_rand((int*)r);
    int64_t a = (int64_t)*r;
    pseudo_rand((int*)r);
    int64_t b = (int64_t)*r;
    return (uint64_t)(a + (b<<32));
}


int32_t pseudo_rand_int(
    unsigned *r, 
    int32_t min, 
    int32_t max)
{
    uint32_t delta = max - min;
    uint32_t d = pseudo_rand_uint32(r) % delta;
    return min + d;
}

uint32_t pseudo_rand_uint(
    unsigned *r, 
    uint32_t min, 
    uint32_t max)
{
    uint32_t delta = max - min;
    uint32_t d = pseudo_rand_uint32(r) % delta;
    return min + d;
}


void pseudo_rand_bytes(unsigned *r, char* buffer, unsigned size){
#ifdef __xcore__
    assert((((unsigned)buffer) & 0x3) == 0);
#endif

    unsigned b = 0;

    while(size >= sizeof(unsigned)){
        pseudo_rand((int*)r);

        char* rch = (char*) r;

        for(int i = 0; i < sizeof(unsigned); i++)
            buffer[b++] = rch[i];

        size -= sizeof(unsigned);
    }
    
    pseudo_rand((int*)r);
    unsigned tmp = *r;
    while(size){
        buffer[b++] = (char) (tmp & 0xFF);
        tmp >>= 8;
        size--;
    }
}
