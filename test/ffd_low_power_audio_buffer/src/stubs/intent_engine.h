// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef INTENT_ENGINE_H_
#define INTENT_ENGINE_H_

#include <stdint.h>
#include <stddef.h>
#include "asr.h"

int32_t intent_engine_sample_push(asr_sample_t *buf, size_t frames);

#endif /* INTENT_ENGINE_H_ */
