// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1
#include <string.h>
#include <xcore/assert.h>

#include "asr.h"


typedef struct mock_asr_struct
{
    int32_t *model;
    uint16_t keyword_id[1];
    int16_t  count;
    uint16_t score;
    int8_t   *dynamic_memory;
} mock_asr_t;

mock_asr_t mock_asr; 

asr_context_t asr_init(int32_t *model, int32_t *grammar) {
    xassert(model);
    xassert(grammar == NULL);

    // read data from the model
    int8_t scratch_data[8];
    ASR_READ_EXT(scratch_data, model, 8); 

    int ret = strncmp((char *)scratch_data, "SIMP-ASR", 8);
    if (ret !=0) {
        ASR_PRINTF("Incorrect model: %s\n", (char *)scratch_data);
        return NULL;
    }

    mock_asr.model = model;
    mock_asr.keyword_id[0] = 100;
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

    // NOTE: We are reading integers from the model file here as strings
    //       You would not typically do this.  It is more typical for commonly
    //       used data to be stored in SRAM and model coeffs to be stored in external 
    //       memory (like flash).  It is highly recommend that coeffs be loaded from 
    //       external memory into SRAM using the ASR_READ_EXT macro before performing
    //       any math with the coeffs.  
    int32_t scratch_data[0];
    ASR_READ_EXT(scratch_data, (const void *)((unsigned)mock_asr->model+12), sizeof(int32_t));
    int16_t sum_threshold = (int16_t)atoi((char *)scratch_data);
    ASR_READ_EXT(scratch_data, (const void *)((unsigned)mock_asr->model+20), sizeof(int32_t));
    int16_t count_threshold = (int16_t)atoi((char *)scratch_data);

    // iterate over all samples and compute sum
    size_t sum = 0;
    for (int i=0; i<buf_len; i++) {
        sum += abs(audio_buf[i]);
    }

    // increment count if sum exceeds threshold
    if (sum > sum_threshold) {
        mock_asr->count++;
    } else {
        mock_asr->count = 0;
    }

    // return keyword if count exceeds threshold
    result->keyword_id = 0;
    result->command_id = 0;
    if (mock_asr->count > count_threshold) {
        result->keyword_id = mock_asr->keyword_id[0];
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

