// Copyright 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef DR_WAV_FREERTOS_PORT_H_
#define DR_WAV_FREERTOS_PORT_H_

/* STD headers */
#include <platform.h>
#include <xs1.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"

/* App headers */
#include "app_conf.h"
#include "platform/driver_instances.h"
#include "fs_support.h"
#include "ff.h"

/* dr_wav Options */
#undef DR_WAV_NO_CONVERSION_API
#define DR_WAV_NO_STDIO

/* dr_wav STD lib port */
// #define DRWAV_ASSERT(expression)    xassert(expression)
// #define DRWAV_MALLOC(sz)            pvPortMalloc((sz))
// #define DRWAV_REALLOC(sz)            pvPortMalloc((sz))
// #define DRWAV_FREE(p)            vPortFree(p)

void *dr_wav_malloc_port(size_t sz, void* pUserData);
void *dr_wav_realloc_port(void* p, size_t sz, void* pUserData);
void dr_wav_free_port(void* p, void* pUserData);

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

drwav_allocation_callbacks drwav_memory_cbs = {
    .pUserData = NULL,
    .onMalloc = dr_wav_malloc_port,
    .onRealloc = dr_wav_realloc_port,
    .onFree = dr_wav_free_port,
};

void *dr_wav_malloc_port(size_t sz, void* pUserData) {
    (void) pUserData;
    rtos_printf("**malloc was called\n");
    return pvPortMalloc(sz);
}

void *dr_wav_realloc_port(void* p, size_t sz, void* pUserData) {
    rtos_printf("**realloc was called\n");
    xassert(0); /* Not implemented in FreeRTOS */
    return NULL;
}

void dr_wav_free_port(void* p, void* pUserData) {
    (void) pUserData;
    rtos_printf("**free was called\n");
    vPortFree(p);
}

size_t drwav_read_proc_port(void* pUserData, void* pBufferOut, size_t bytesToRead) {
    FIL *file = (FIL*)pUserData;
    FRESULT result;
    uint32_t bytes_read = 0;

    result = f_read(file,
                    (uint8_t*)pBufferOut,
                    bytesToRead,
                    (unsigned int*)&bytes_read);

    return (result == FR_OK) ? bytes_read : 0;
}

size_t drwav_write_proc_port(void* pUserData, const void* pData, size_t bytesToWrite) {
    rtos_printf("drwav_write_proc_port not implemented\n");
    return 0;
}

drwav_bool32 drwav_seek_proc_port(void* pUserData, int offset, drwav_seek_origin origin) {
    FIL *file = (FIL*)pUserData;
    FRESULT result;

    result = f_lseek(file, offset);

    return (result == FR_OK) ? DRWAV_TRUE : DRWAV_FALSE;;
}

drwav_uint64 drwav_chunk_proc_port(void* pChunkUserData, drwav_read_proc onRead, drwav_seek_proc onSeek, void* pReadSeekUserData, const drwav_chunk_header* pChunkHeader, drwav_container container, const drwav_fmt* pFMT) {
    rtos_printf("drwav_chunk_proc_port not implemented\n");
    return (drwav_uint64)-1;
}

#endif /* DR_WAV_FREERTOS_PORT_H_ */
