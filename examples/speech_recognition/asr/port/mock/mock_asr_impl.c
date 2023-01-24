// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#include "asr.h"

#include <math.h>
#include <xcore/assert.h>


typedef struct mock_asr_struct
{
    uint16_t keyword_id;
    int16_t  sum_threshold;
    int16_t  count_threshold;
    int16_t  count;
    uint16_t score;
    int8_t   *dynamic_memory;
} mock_asr_t;

mock_asr_t mock_asr; 

asr_context_t asr_init() {

    mock_asr.keyword_id = 100;
    mock_asr.sum_threshold = 1000;
    mock_asr.count_threshold = 75;
    mock_asr.count = 0;
    mock_asr.score = INT16_MAX;

    // allocate some mock dynamic memory using the defined allocator macros
    mock_asr.dynamic_memory = (int8_t *)ASR_MALLOC(1024);    

    return (asr_context_t) &mock_asr;
}

asr_error_t asr_process(asr_context_t *ctx, int16_t *audio_buf, size_t buf_len, asr_result_t *result)
{
    xassert(ctx);

    mock_asr_t *mock_asr = (mock_asr_t *) ctx;

    // example of using the ASR read ext macro to read data from extended memory (flash in this example)
    int8_t dest[100];
    ASR_READ_EXT(dest, (void *)0x1234, 100);  // 0x1234 is an arbitrary offset in the flash address space

    // iterate over all samples and compute sum
    size_t sum = 0;
    for (int i=0; i<buf_len; i++) {
        sum += abs(audio_buf[i]);
    }

    // increment count if sum exceeds threshold
    if (sum > mock_asr->sum_threshold) {
        mock_asr->count++;
    } else {
        mock_asr->count = 0;
    }

    // return keyword if count exceeds threshold
    result->keyword_id = 0;
    result->command_id = 0;
    if (mock_asr->count > mock_asr->count_threshold) {
        result->keyword_id = mock_asr->keyword_id;
        mock_asr->count = 0;
    }

    return ASR_OK;
}

asr_error_t asr_reset(asr_context_t *ctx)
{
    xassert(ctx);

    mock_asr_t *mock_asr = (mock_asr_t *) ctx;
    mock_asr->count = 0;

    return ASR_OK;
}

asr_error_t asr_release(asr_context_t *ctx)
{
    xassert(ctx);

    mock_asr_t *mock_asr = (mock_asr_t *) ctx;

    // free the mock dynamic memory
    ASR_FREE((void *)mock_asr->dynamic_memory);

    ctx = NULL;

    return ASR_OK;
}

asr_keyword_t asr_get_keyword(asr_context_t *ctx, int16_t asr_id)
{
    switch (asr_id) {
        case 100:
            return ASR_KEYWORD_HELLO_XMOS;
            break;
        default:
            break ;
    }

    return ASR_KEYWORD_UNSUPPORTED;
}

asr_command_t asr_get_command(asr_context_t *ctx, int16_t asr_id)
{
    // This asr port does not support any commands
    return ASR_COMMAND_UNSUPPORTED;
}

