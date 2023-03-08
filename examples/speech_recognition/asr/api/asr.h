// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef XCORE_VOICE_ASR_H
#define XCORE_VOICE_ASR_H

#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

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
 * ASR ports should call asr_malloc instead of malloc
 */
__attribute__((weak))
void * asr_malloc(size_t size) {
    return malloc(size);
}

/**
 * Memory deallocation macro that allows the application 
 * to provide an alternative implementation.
 * 
 * ASR ports should call asr_free instead of free
 */
__attribute__((weak))
void asr_free(void *ptr) {
    free(ptr);
}

// Extentended  macro that allows the application 
// to provide an alternative implementation.
//
// ASR ports should call ASR_FREE instead of free

/**
 * Synchronous extended memory read macro that allows the application  
 * to provide an alternative implementation.  Blocks the callers thread 
 * until the read is completed.
 * 
 * ASR ports should call asr_read_ext instead of any other
 * functions to read memory from flash, LPDDR or SDRAM. 
 * ASR ports are free to use memcpy if the dest and src 
 * are both SRAM addresses.  
 */
__attribute__((weak))
void asr_read_ext(void *dest, const void * src, size_t n) {
    memcpy(dest, src, n);
}

/**
 * Asynchronous extended memory read macro that allows the application  
 * to provide an alternative implementation.
 * 
 * ASR ports should call asr_read_ext_async instead of any other
 * functions to read memory from flash, LPDDR or SDRAM. 
 */
__attribute__((weak))
int asr_read_ext_async(void *dest, const void * src, size_t n) {
    memcpy(dest, src, n);
    return 0;
}

/**
 * Wait in the caller's thread for an asynchronous extended memory read to finish.
 */
__attribute__((weak))
void asr_read_ext_wait(int handle) {
    return;
}

/**
 * String output macro that allows the application  
 * to provide an alternative implementation.
 * 
 * ASR ports should call asr_printf instead of printf
 */
__attribute__((weak))
void asr_printf(const char * format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

/**
 * Typedef to the ASR port context struct.
 *
 * An ASR port can store any data needed in the context.
 * The context pointer is passed to all API methods and 
 * can be cast to any struct defined by the ASR port.
 */
typedef void* asr_context_t;

/**
 * Typedef to the ASR port and model attributes
 */
typedef struct asr_attributes_struct
{
    int16_t     samples_per_brick;  ///< Input brick length (in samples) required for calls to asr_process
    const char* engine_version;     ///< ASR port engine version
    const char* model_version;      ///< Model version
    size_t      required_memory;    ///< Memory (in bytes) required by engine and model
    void*       reserved;           ///< Reserved for future use
} asr_attributes_t;

/**
 * Typedef to the ASR result
 */
typedef struct asr_result_struct
{
    uint16_t keyword_id;     ///< Keyword ID
    uint16_t command_id;     ///< Command ID
    int16_t  score;          ///< The confidence score of the detection
    uint16_t gscore;         ///< The garbage score
    void*    reserved;       ///< Reserved for future use
} asr_result_t;

/**
 * Enumerator type representing error return values.
 */
typedef enum asr_error_enum {
    ASR_OK = 0,              ///< Ok
    ASR_ERROR,               ///< General error  
    ASR_INSUFFICIENT_MEMORY, ///< Insufficient memory for given model
    ASR_NOT_SUPPORTED,       ///< Function not supported for given model
    ASR_INVALID_PARAMETER,   ///< Invalid Parameter
    ASR_MODEL_INCOMPATIBLE,  ///< Model type or version is not compatible with the ASR library
    ASR_MODEL_CORRUPT,       ///< Model malformed
    ASR_NOT_INITIALIZED,     ///< Not Initialized
    ASR_EVALUATION_EXPIRED,   ///< Evaluation period has expired
} asr_error_t;

/**
 * Enumerator type representing each supported keyword.
 */
typedef enum asr_keyword_enum {
    ASR_KEYWORD_UNKNOWN = -1,    ///< Keyword is unknown
    ASR_KEYWORD_HELLO_XMOS = 0,
    ASR_KEYWORD_ALEXA = 1,
    ASR_NUMBER_OF_KEYWORDS
} asr_keyword_t;

/**
 * Enumerator type representing each supported command.
 */
typedef enum asr_command_enum {
    ASR_COMMAND_UNKNOWN = -1,    ///< Command is unknown
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
    ASR_NUMBER_OF_COMMANDS
} asr_command_t;

/**
 * Initialize an ASR port.
 *
 * \param model      A pointer to the model data.
 * \param grammar    A pointer to the grammar data (Optional).
 *
 * \returns the ASR context.
 */
asr_context_t asr_init(int32_t *model, int32_t *grammar);

/**
 * Get engine and model attributes.
 *
 * \param ctx         A pointer to the ASR context.
 * \param attributes  The attributes result.
 * 
 * \returns Success or error code.  
 */
asr_error_t asr_get_attributes(asr_context_t *ctx, asr_attributes_t *attributes);

/**
 * Process an audio buffer.
 *
 * \param ctx        A pointer to the ASR context.
 * \param audio_buf  A pointer to the 16-bit PCM samples.
 * \param buf_len    The number of PCM samples.
 * 
 * \returns Success or error code.  
 */
asr_error_t asr_process(asr_context_t *ctx, int16_t *audio_buf, size_t buf_len);

/**
 * Get the most recent results.
 *
 * \param ctx        A pointer to the ASR context.
 * \param result     The processed result.
 * 
 * \returns Success or error code.  
 */
asr_error_t asr_get_result(asr_context_t *ctx, asr_result_t *result);

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
