// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1
#include <string.h>
#include <xcore/assert.h>

#include "asr.h"
#include "rtos_swmem.h"
#include "wanson_api.h"

#define MAX_EVAL_RECOGNITIONS  (50)

typedef struct wanson_asr_struct
{
    int      recog_id;
    int16_t  recog_count;
} wanson_asr_t;

wanson_asr_t wanson_asr; 

asr_port_t asr_init(int32_t *model, int32_t *grammar, devmem_manager_t *devmem_ctx) {
    xassert(model == NULL);
    xassert(grammar == NULL);

    // NOTE: The Wanson asr port uses the .SwMem_data attribute but no SwMem event handling code is required.
    //       This may cause xflash to whine if the compiler optimizes out the __swmem_address symbol.
    //       To work around this, we simply need to init the swmem.
    rtos_swmem_init(0);

    asr_printf("Wanson init\n");

    Wanson_ASR_Init();
    wanson_asr.recog_count = 0;

    asr_printf("Wanson init done\n");

    return (asr_port_t) &wanson_asr;
}

asr_error_t asr_get_attributes(asr_port_t *ctx, asr_attributes_t *attributes) {
    xassert(ctx);

    return ASR_NOT_SUPPORTED;
}

asr_error_t asr_process(asr_port_t *ctx, int16_t *audio_buf, size_t buf_len)
{
    xassert(ctx);
    xassert(buf_len == 480); // Wanson engine requires 480 samples per block

    wanson_asr_t *wanson_asr = (wanson_asr_t *) ctx;
    char *text_ptr = NULL;

    wanson_asr->recog_id = 0;
    int ret = Wanson_ASR_Recog(audio_buf, buf_len, (const char **)&text_ptr, &wanson_asr->recog_id);

    if (ret == 1) {
        wanson_asr->recog_count++;
    } else if (ret < 0) {
        asr_printf("Wanson recog: ret=%d\n", ret);
        return ASR_ERROR;
    } 

    if (wanson_asr->recog_count > MAX_EVAL_RECOGNITIONS) {
        asr_printf("Wanson eval expired\n");
        return ASR_EVALUATION_EXPIRED;
    }

    return ASR_OK;
}

asr_error_t asr_get_result(asr_port_t *ctx, asr_result_t *result) {
    xassert(ctx);
    xassert(result);

    wanson_asr_t *wanson_asr = (wanson_asr_t *) ctx;

    result->id = wanson_asr->recog_id;

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
    wanson_asr.recog_id = 0;

    // domain doesn't do anything right now, 0 is both wakeup and asr
    asr_printf("Wanson reset for wakeup\n");
    int ret = Wanson_ASR_Reset(0);
    asr_printf("Wanson reset ret: %d\n", ret);

    return ASR_OK;
}

asr_error_t asr_release(asr_port_t *ctx)
{
    Wanson_ASR_Release();
    ctx = NULL;

    return ASR_OK;
}
