// Copyright (c) 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public License: Version 1

#ifndef WAKE_WORD_ENGINE_H_
#define WAKE_WORD_ENGINE_H_

#include <stdint.h>
#include <stddef.h>
#include "app_conf.h"
#include "asr.h"

typedef enum {
    WAKEWORD_FOUND,
    WAKEWORD_NOT_FOUND,
    WAKEWORD_ERROR
} wakeword_result_t;

void wake_word_engine_init(void);
wakeword_result_t wake_word_engine_handler(asr_sample_t *buf, size_t num_frames);

#endif /* WAKE_WORD_ENGINE_H_ */
