// Copyright (c) 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public License: Version 1

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include "inference_engine.h"

// Defined by test logic.
void verify_inference_engine_sample_push_args(int32_t *buf, size_t frames);

/* Stub for inference_engine_sample_push */
int32_t inference_engine_sample_push(int32_t *buf, size_t frames)
{
    verify_inference_engine_sample_push_args(buf, frames);
    return 0;
}