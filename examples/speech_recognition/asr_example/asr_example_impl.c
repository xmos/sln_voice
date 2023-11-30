// Copyright 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <string.h>

#include <xcore/assert.h>

#include "asr.h"

typedef struct mock_asr_struct
{
    int32_t *model;
    uint16_t word_id[1];
    int16_t  count;
    uint16_t score;
    uint16_t spotted_word_id;
    int8_t   *dynamic_memory;
    devmem_manager_t *devmem_ctx;
} mock_asr_t;

mock_asr_t mock_asr; 

asr_port_t asr_init(int32_t *model, int32_t *grammar, devmem_manager_t *devmem_ctx) {
    xassert(grammar == NULL);

    int8_t scratch_data[8];
    
    // set port data 
    mock_asr.model = model;
    mock_asr.word_id[0] = 100;
    mock_asr.spotted_word_id = 0;
    mock_asr.count = 0;
    mock_asr.score = INT16_MAX;
    mock_asr.devmem_ctx = devmem_ctx;

    // example of how to read data from the model
    devmem_read_ext(mock_asr.devmem_ctx, scratch_data, model, 8);

    // example of how to allocate some mock dynamic memory 
    // using the asr_malloc function
    mock_asr.dynamic_memory = (int8_t *)devmem_malloc(mock_asr.devmem_ctx, 1024);    

    int ret = strncmp((char *)scratch_data, "SIMP-ASR", 8);
    if (ret !=0) {
        asr_printf("Incorrect model: %s\n", (char *)scratch_data);
        return NULL;
    }

    return (asr_port_t) &mock_asr;
}

asr_error_t asr_get_attributes(asr_port_t *ctx, asr_attributes_t *attributes) {
    xassert(ctx);

    return ASR_NOT_SUPPORTED;
}

asr_error_t asr_process(asr_port_t *ctx, int16_t *audio_buf, size_t buf_len)
{
    xassert(ctx);

    mock_asr_t *mock_asr = (mock_asr_t *) ctx;

    // NOTE: We are reading integers from the model file here as strings
    //       You would not typically do this.  It is more typical for commonly
    //       used data to be stored in SRAM and model coeffs to be stored in external 
    //       memory (like flash).  It is highly recommend that coeffs be loaded from 
    //       external memory into SRAM using the asr_read_ext or asr_read_ext_async 
    //       functions before performing any math with the coeffs.  
    int wait_handle; 
    int32_t scratch_data[0];

    // read data from the model in another thread
    wait_handle = devmem_read_ext_async(mock_asr->devmem_ctx, scratch_data, (const void *)((unsigned)mock_asr->model+12), sizeof(int32_t));
    
    // could do some other work here

    // block until read is finished, then do something with the data
    devmem_read_ext_wait(mock_asr->devmem_ctx, wait_handle);
    int16_t sum_threshold = (int16_t)atoi((char *)scratch_data);

    // could do some other work here

    // read data from the model in another thread
    wait_handle = devmem_read_ext_async(mock_asr->devmem_ctx, scratch_data, (const void *)((unsigned)mock_asr->model+20), sizeof(int32_t));

    // block until read is finished, then do something with the data
    devmem_read_ext_wait(mock_asr->devmem_ctx, wait_handle);
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
    mock_asr->spotted_word_id = 0;
    if (mock_asr->count > count_threshold) {
        mock_asr->spotted_word_id = mock_asr->word_id[0]; // 0 is the only supported ID in this oversimplified example
        mock_asr->count = 0;
    }

    return ASR_OK;
}

asr_error_t asr_get_result(asr_port_t *ctx, asr_result_t *result) {
    xassert(ctx);

    mock_asr_t *mock_asr = (mock_asr_t *) ctx;

    result->id = mock_asr->spotted_word_id;

    // The following result fields are not implemented
    result->score = 0;
    result->gscore = 0;
    result->start_index = -1;
    result->end_index = -1;
    result->duration = -1;
    
    return ASR_OK;
}

asr_error_t asr_reset(asr_port_t *ctx)
{
    xassert(ctx);

    mock_asr_t *mock_asr = (mock_asr_t *) ctx;
    mock_asr->count = 0;

    return ASR_OK;
}

asr_error_t asr_release(asr_port_t *ctx)
{
    xassert(ctx);

    mock_asr_t *mock_asr = (mock_asr_t *) ctx;

    // free the mock dynamic memory
    devmem_free(mock_asr->devmem_ctx, (void *)mock_asr->dynamic_memory);

    ctx = NULL;

    return ASR_OK;
}
