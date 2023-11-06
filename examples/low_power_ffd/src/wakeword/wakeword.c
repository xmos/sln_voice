// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* This unit is responsible for waking up the other tile while in low power
 * mode, the model used in this unit needs to only detect the wake word.
 * Requests to enter low power mode are to be made by the other tile. */

/* STD headers */
#include <platform.h>
#include <xs1.h>

/* App headers */
#include "app_conf.h"
#include "asr.h"
#include "device_memory_impl.h"
#include "wakeword/wakeword.h"
#include "platform/driver_instances.h"

// This define is referenced by the model source/header files.
#ifndef ALIGNED
#define ALIGNED(x) __attribute__ ((aligned((x))))
#endif

#define WAKEWORD_NET_VAR       dnn_wakeword_netLabel
#define WAKEWORD_SEARCH_VAR    gs_wakeword_grammarLabel

#ifdef WAKEWORD_NET_SOURCE_FILE
#include WAKEWORD_NET_SOURCE_FILE
#else
extern const unsigned short WAKEWORD_NET_VAR[];
#endif

#ifdef WAKEWORD_SEARCH_HEADER_FILE
#include WAKEWORD_SEARCH_HEADER_FILE
#endif

#ifdef WAKEWORD_SEARCH_SOURCE_FILE
#include WAKEWORD_SEARCH_SOURCE_FILE
#else
extern const unsigned short WAKEWORD_SEARCH_VAR[];
#endif

#define WAKEWORD_PHRASE    "Hello XMOS"
#define WAKEWORD_ID        0x1
#define IS_WAKEWORD(id)    ((id) == (WAKEWORD_ID))

#define SAMPLES_PER_ASR     (appconfINTENT_SAMPLE_BLOCK_LENGTH)

static asr_port_t asr_ctx;
static devmem_manager_t devmem_ctx;

void wakeword_init(void)
{
    devmem_init(&devmem_ctx);
    asr_ctx = asr_init((int32_t *)WAKEWORD_NET_VAR, (int32_t *)WAKEWORD_SEARCH_VAR, &devmem_ctx);
    asr_reset(asr_ctx);
}

wakeword_result_t wakeword_handler(asr_sample_t *buf, size_t num_frames)
{
    asr_result_t asr_result;
    asr_error_t asr_error;
    wakeword_result_t retval = WAKEWORD_NOT_FOUND;

    asr_error = asr_process(asr_ctx, buf, num_frames);

    if (asr_error == ASR_OK) {
        asr_error = asr_get_result(asr_ctx, &asr_result);
    }

    if (asr_error == ASR_EVALUATION_EXPIRED) {
        retval = WAKEWORD_ERROR;
    } else if (asr_error != ASR_OK) {
        debug_printf("ASR error on tile %d: %d\n", THIS_XCORE_TILE, asr_error);
    } else if (IS_WAKEWORD(asr_result.id)) {
        debug_printf("KEYWORD: " RTOS_STRINGIFY(WAKEWORD_ID) ", " WAKEWORD_PHRASE "\n");
        retval = WAKEWORD_FOUND;
    }
    return retval;
}
