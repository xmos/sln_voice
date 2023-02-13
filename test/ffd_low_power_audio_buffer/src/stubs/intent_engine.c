// Copyright (c) 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public License: Version 1

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include "intent_engine.h"

void print_data(int32_t* buffer, size_t count)
{
    printf("OUTPUT DATA --> ");

    for (size_t i = 0; i < count; i++)
        printf("%3ld ", buffer[i]);

    printf("\n");
}

/* Stub for intent_engine_sample_push */
int32_t intent_engine_sample_push(int32_t *buf, size_t frames)
{
    print_data(buf, frames);
    return 0;
}