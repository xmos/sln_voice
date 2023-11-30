// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef WAKEWORD_ENGINE_H_
#define WAKEWORD_ENGINE_H_

#include <stdint.h>
#include <stddef.h>
#include "app_conf.h"
#include "asr.h"

typedef enum {
    WAKEWORD_FOUND,
    WAKEWORD_NOT_FOUND,
    WAKEWORD_ERROR
} wakeword_result_t;

void wakeword_init(void);
wakeword_result_t wakeword_handler(asr_sample_t *buf, size_t num_frames);

#endif /* WAKEWORD_ENGINE_H_ */
