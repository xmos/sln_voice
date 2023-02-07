// Copyright (c) 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public License: Version 1

#ifndef INFERENCE_ENGINE_H_
#define INFERENCE_ENGINE_H_

#include <stdint.h>
#include <stddef.h>

int32_t inference_engine_sample_push(int32_t *buf, size_t frames);

#endif /* INFERENCE_ENGINE_H_ */
