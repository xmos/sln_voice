// Copyright (c) 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public License: Version 1

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
#include "wakeword/wake_word_engine.h"
#include "platform/driver_instances.h"

// This define is referenced by the model source/header files.
#ifndef ALIGNED
#define ALIGNED(x) __attribute__ ((aligned((x))))
#endif

#define WAKE_WORD_NET_VAR       dnn_wakeword_netLabel
#define WAKE_WORD_SEARCH_VAR    gs_wakeword_grammarLabel

#ifdef WAKE_WORD_NET_SOURCE_FILE
#include WAKE_WORD_NET_SOURCE_FILE
#else
extern const unsigned short WAKE_WORD_NET_VAR[];
#endif

#ifdef WAKE_WORD_SEARCH_HEADER_FILE
#include WAKE_WORD_SEARCH_HEADER_FILE
#endif

#ifdef WAKE_WORD_SEARCH_SOURCE_FILE
#include WAKE_WORD_SEARCH_SOURCE_FILE
#else
extern const unsigned short WAKE_WORD_SEARCH_VAR[];
#endif

#define WAKE_WORD_PHRASE    "Hello XMOS"
#define WAKE_WORD_ID        0x1
#define IS_WAKE_WORD(id)    ((id) == (WAKE_WORD_ID))

#define SAMPLES_PER_ASR     (appconfINTENT_SAMPLE_BLOCK_LENGTH)

static asr_port_t asr_ctx;
static devmem_manager_t devmem_ctx;

void wake_word_engine_init(void)
{
    devmem_init(&devmem_ctx);
    asr_ctx = asr_init((int32_t *)WAKE_WORD_NET_VAR, (int32_t *)WAKE_WORD_SEARCH_VAR, &devmem_ctx);
    asr_reset(asr_ctx);
}

wakeword_result_t wake_word_engine_handler(asr_sample_t *buf, size_t num_frames)
{
    asr_result_t asr_result;
    asr_error_t asr_error;
    wakeword_result_t retval = WAKEWORD_NOT_FOUND;

    asr_error = asr_process(asr_ctx, buf, num_frames);

    if (asr_error == ASR_OK) {
        asr_error = asr_get_result(asr_ctx, &asr_result);
    }

    if (asr_error == ASR_EVALUATION_EXPIRED) {
<<<<<<< HEAD:examples/low_power_ffd/src/wakeword/wake_word_engine.c
        retval = WAKEWORD_ERROR;
=======
        power_control_halt();
        power_state_set(POWER_STATE_FULL);
>>>>>>> develop:examples/low_power_ffd/src/intent_engine/wake_word_engine.c
    } else if (asr_error != ASR_OK) {
        debug_printf("ASR error on tile %d: %d\n", THIS_XCORE_TILE, asr_error);
    } else if (IS_WAKE_WORD(asr_result.id)) {
        debug_printf("KEYWORD: " RTOS_STRINGIFY(WAKE_WORD_ID) ", " WAKE_WORD_PHRASE "\n");
        retval = WAKEWORD_FOUND;
    }
    return retval;
}
