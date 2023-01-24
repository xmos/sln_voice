// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef XCORE_VOICE_ASR_H
#define XCORE_VOICE_ASR_H

#include <stdint.h>
#include <stdlib.h>

#include "asr_config.h" // application provided configuration

/**
 * \addtogroup asr_api asr_api
 *
 * The public API for XCORE-VOICE ASR ports.
 * @{
 */

/**
 * Memory allocation macro that allows the application 
 * to provide an alternative implementation.
 * 
 * ASR ports should call ASR_MALLOC instead of malloc
 */
#ifndef ASR_MALLOC
#define ASR_MALLOC      malloc
#endif

/**
 * Memory deallocation macro that allows the application 
 * to provide an alternative implementation.
 * 
 * ASR ports should call ASR_FREE instead of free
 */
#ifndef ASR_FREE
#define ASR_FREE        free
#endif

// Extentended  macro that allows the application 
// to provide an alternative implementation.
//
// ASR ports should call ASR_FREE instead of free

/**
 * Extended memory read macro that allows the application  
 * to provide an alternative implementation.
 * 
 * ASR ports should call ASR_READ_EXT instead of any other
 * functions to read memory from flash, LPDDR or SDRAM. 
 * ASR ports are free to use memcpy if the dest and src 
 * are both SRAM addresses.  
 */
#ifndef ASR_READ_EXT
#define ASR_READ_EXT    memcpy
#endif

/**
 * String output macro that allows the application  
 * to provide an alternative implementation.
 * 
 * ASR ports should call ASR_PRINTF instead of printf
 */
#ifndef ASR_PRINTF
#define ASR_PRINTF(...) printf(__VA_ARGS__)
#endif


/**
 * Typedef to the ASR port context struct.
 *
 * An ASR port can store any data needed in the context.
 * The context pointer is passed to all API methods and 
 * can be cast to any struct defined by the ASR port.
 */
typedef void* asr_context_t;

/**
 * Typedef to the ASR process API return.
 */
typedef struct asr_result_struct
{
    uint16_t keyword_id;     // keyword ID
    uint16_t command_id;     // command ID
    int16_t  score;          // The confidence score of the detection
    uint16_t gscore;         // The garbage score
} asr_result_t;

/**
 * Enumerator type representing error return values.
 */
typedef enum asr_error_enum {
    ASR_OK = 0,
    ASR_ERROR,
} asr_error_t;

/**
 * Enumerator type representing each supported keyword.
 */
typedef enum asr_keyword_enum {
    ASR_KEYWORD_UNSUPPORTED = -1,
    ASR_KEYWORD_HELLO_XMOS = 0,
    ASR_KEYWORD_ALEXA = 1,
} asr_keyword_t;

/**
 * Enumerator type representing each supported command.
 */
typedef enum asr_command_enum {
    ASR_COMMAND_UNSUPPORTED = -1,
    ASR_COMMAND_TV_ON = 0,
    ASR_COMMAND_TV_OFF = 1,
    ASR_COMMAND_VOLUME_UP = 2,
    ASR_COMMAND_VOLUME_DOWN = 3,
    ASR_COMMAND_CHANNEL_UP = 4,
    ASR_COMMAND_CHANNEL_DOWN = 5,
    ASR_COMMAND_LIGHTS_ON = 6,
    ASR_COMMAND_LIGHTS_OFF = 7,
    ASR_COMMAND_LIGHTS_UP = 8,
    ASR_COMMAND_LIGHTS_DOWN = 9,
    ASR_COMMAND_FAN_ON = 10,
    ASR_COMMAND_FAN_OFF = 11,
    ASR_COMMAND_FAN_UP = 12,
    ASR_COMMAND_FAN_DOWN = 13,
    ASR_COMMAND_TEMPERATURE_UP = 14,
    ASR_COMMAND_TEMPERATURE_DOWN = 15,
} asr_command_t;

/**
 * Initialize an ASR port.
 *
 * \returns the ASR context.
 */
asr_context_t asr_init();

/**
 * Process an audio buffer.
 *
 * \param ctx        A pointer to the ASR context.
 * \param audio_buf  A pointer to the 16-bit PCM samples.
 * \param buf_len    The number of PCM samples.
 * \param result     The processed result.
 * 
 * \returns Success or error code.  
 */
asr_error_t asr_process(asr_context_t *ctx, int16_t *audio_buf, size_t buf_len, asr_result_t *result);

/**
 * Reset ASR port (if necessary).
 * 
 * Called before the next call to asr_process.
 *
 * \param ctx        A pointer to the ASR context.
 * 
 * \returns Success or error code.  
 */
asr_error_t asr_reset(asr_context_t *ctx);

/**
 * Release ASR port (if necessary).
 * 
 * The ASR port must deallocate any memory.
 *
 * \param ctx        A pointer to the ASR context.
 * 
 * \returns Success or error code.  
 */
asr_error_t asr_release(asr_context_t *ctx);

/**
 * Return the XCORE-VOICE supported keyword type.
 *
 * \param ctx        A pointer to the ASR context.
 * \param asr_id     The ASR port keyword identifier.
 * 
 * \returns XCORE-VOICE supported keyword type  
 */
asr_keyword_t asr_get_keyword(asr_context_t *ctx, int16_t asr_id);

/**
 * Return the XCORE-VOICE supported command type.
 *
 * \param ctx        A pointer to the ASR context.
 * \param asr_id     The ASR port command identifier.
 * 
 * \returns XCORE-VOICE supported command type  
 */
asr_command_t asr_get_command(asr_context_t *ctx, int16_t asr_id);

/**@}*/

#endif // XCORE_VOICE_ASR_H
