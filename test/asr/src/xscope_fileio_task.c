// Copyright 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <platform.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "soc_xscope_host.h"

#include "app_conf.h"
#include "asr.h"
#include "device_memory_impl.h"
#include "platform/driver_instances.h"
#include "wav_utils.h"
#include "xscope_fileio_task.h"
#include "xscope_io_device.h"

#ifndef DWORD_ALIGNED
#define DWORD_ALIGNED     __attribute__ ((aligned(8)))
#endif

#if (appconfASR_LIBRARY_ID == 0)
    // Sensory
    
    // SEARCH model file is specified in the CMakeLists SENSORY_SEARCH_FILE variable
    extern const unsigned short gs_grammarLabel[];
    void* grammar = (void*)gs_grammarLabel;

    // Model file is in flash at the offset specified in the CMakeLists
    // QSPI_FLASH_MODEL_START_ADDRESS variable.  The XS1_SWMEM_BASE value needs
    // to be added so the address in in the SwMem range.  
    uint16_t *model = (uint16_t *) (XS1_SWMEM_BASE + QSPI_FLASH_MODEL_START_ADDRESS);
#else
    #error "Unsupported appconfASR_LIBRARY_ID"
#endif

static TaskHandle_t xscope_fileio_task_handle;
static xscope_file_t infile;
static xscope_file_t outfile;
static asr_port_t asr_ctx; 
static devmem_manager_t devmem_ctx;
static char log_buffer[1024];

#if ON_TILE(XSCOPE_HOST_IO_TILE)
static SemaphoreHandle_t mutex_xscope_fileio;

void xscope_fileio_lock_alloc(void) {
    mutex_xscope_fileio = xSemaphoreCreateMutex();
    xassert(mutex_xscope_fileio);
}

void xscope_fileio_lock_acquire(void) {
    xSemaphoreTake(mutex_xscope_fileio, portMAX_DELAY);
}

void xscope_fileio_lock_release(void) {
    xSemaphoreGive(mutex_xscope_fileio);
}

void init_xscope_host_data_user_cb(chanend_t c_host) {
    xscope_io_init(c_host);
}
#endif

void xscope_fileio_user_done(void) {
    xTaskNotifyGive(xscope_fileio_task_handle);
}

/* This task reads the input file in chunks and sends it through the ASR engine
 * After reading the entire file, it will wait until the user has confirmed
 * all writing is complete before closing files.
 */
 /* NOTE:
  * xscope fileio uses events.  Only xscope_fread() currently, but wrapping
  * all calls in a critical section just in case */
#pragma stackfunction 5000
void xscope_fileio_task(void *arg) {
    (void) arg;
    int state = 0;
    wav_header input_header_struct;
    unsigned input_header_size;
    unsigned frame_count;
    unsigned brick_count;        
    uint32_t DWORD_ALIGNED in_buf_raw_32[appconfASR_BRICK_SIZE_SAMPLES * appconfINPUT_CHANNELS];
    int16_t DWORD_ALIGNED in_buf_int_16[appconfINPUT_CHANNELS * appconfASR_BRICK_SIZE_SAMPLES];
    size_t bytes_read = 0;

    /* Wait until xscope_fileio is initialized */
    while(xscope_fileio_is_initialized() == 0) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    rtos_printf("Opening files\n");
    state = rtos_osal_critical_enter();
    {
        infile = xscope_open_file(appconfINPUT_FILENAME, "rb");
        // Validate input wav file
        if(get_wav_header_details(&infile, &input_header_struct, &input_header_size) != 0){
            rtos_printf("Error: error in get_wav_header_details()\n");
            _Exit(1);
        }
        xscope_fseek(&infile, input_header_size, SEEK_SET);
        vTaskDelay(pdMS_TO_TICKS(1000));
        outfile = xscope_open_file(appconfOUTPUT_FILENAME, "wt");
    }
    rtos_osal_critical_exit(state);

    // Ensure 32bit wav file
    if(input_header_struct.bit_depth != appconfSAMPLE_BIT_DEPTH)
    {
        rtos_printf("Error: unsupported wav bit depth (%d) for %s file. Only 32 supported\n", input_header_struct.bit_depth, appconfINPUT_FILENAME);
        _Exit(1);
    }
    // Ensure input wav file contains correct number of channels 
    if(input_header_struct.num_channels != appconfINPUT_CHANNELS){
        rtos_printf("Error: wav num channels(%d) does not match (%u)\n", input_header_struct.num_channels, appconfINPUT_CHANNELS);
        _Exit(1);
    }
    
    // Calculate number of frames in the wav file
    frame_count = wav_get_num_frames(&input_header_struct);
    brick_count = frame_count / appconfASR_BRICK_SIZE_SAMPLES; 

    // Init ASR library
    devmem_init(&devmem_ctx);
    asr_ctx = asr_init((void *) model, (void *) grammar, &devmem_ctx);
    asr_reset(asr_ctx);

    asr_error_t asr_error;
    asr_result_t asr_result;

    memset(&asr_error, 0, sizeof(asr_error_t));

    rtos_printf("Processing %d bricks\n", brick_count);

    // Iterate over audio bricks
    for(unsigned b=0; b<brick_count; b++) {
        memset(in_buf_raw_32, 0, appconfASR_BRICK_SIZE_BYTES);
        long input_location =  wav_get_frame_start(&input_header_struct, b * appconfASR_BRICK_SIZE_SAMPLES, input_header_size);

        if (b % 100 == 0) {
            rtos_printf("Processing brick %d of %d\n", b, brick_count);
        }

        // Read from input wav file
        state = rtos_osal_critical_enter();
        {
            xscope_fseek(&infile, input_location, SEEK_SET);
            bytes_read = xscope_fread(&infile, (uint8_t *) &in_buf_raw_32[0], appconfASR_BRICK_SIZE_BYTES);
        }
        rtos_osal_critical_exit(state);

        // De-interleave input and convert to 16-bit
        //  wav files are in frame-major order, pipeline expects sample-major order
        for(unsigned f=0; f<appconfASR_BRICK_SIZE_SAMPLES; f++){
            for(unsigned ch=0; ch<appconfINPUT_CHANNELS; ch++) {
                in_buf_int_16[ch * appconfASR_BRICK_SIZE_SAMPLES + f] = in_buf_raw_32[f * appconfINPUT_CHANNELS + ch] >> 16;
            }
        }

        // Send audio to ASR
        asr_error = asr_process(asr_ctx, in_buf_int_16, appconfASR_BRICK_SIZE_SAMPLES);
        if (asr_error != ASR_OK) continue; 

        asr_error = asr_get_result(asr_ctx, &asr_result);
        if (asr_error != ASR_OK) continue; 

        // Query or compute recognition event metadata
        size_t start_index;
        size_t end_index;
        size_t duration;
        
        if (asr_result.id > 0) {
            if (asr_result.start_index > 0) {
                start_index = asr_result.start_index;
            } else {
                // No metadata so assume this brick - appconfASR_MISSING_START_METADATA_CORRECTION
                start_index = (b * appconfASR_BRICK_SIZE_SAMPLES) - appconfASR_MISSING_METADATA_CORRECTION; 
            }

            if (asr_result.end_index > 0) {
                end_index = asr_result.end_index;        
            } else {
                // No metadata so assume start_index
                end_index = start_index; 
            }

            if (asr_result.duration > 0) {
                duration = asr_result.duration;        
            } else {
                // No metadata so assume no duration
                duration = 0;        
            }

            // Log result
            sprintf(log_buffer, "RECOGNIZED: id=%d, start=%d, end=%d, duration=%d\n", 
                asr_result.id,
                start_index,
                end_index,
                duration
            );
            rtos_printf(log_buffer);
            xscope_fwrite(&outfile, (uint8_t *)&log_buffer[0], strlen(log_buffer));
        }
    }

#if (appconfAPP_NOTIFY_FILEIO_DONE == 1)
    /* Wait for user to tell us they are done writing */
    (void) ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
#else
    /* Otherwise, assume data pipeline is done after 1 second */
    vTaskDelay(pdMS_TO_TICKS(1000));
#endif

    rtos_printf("Close all files\n");
    state = rtos_osal_critical_enter();
    {
        xscope_close_all_files();
    }
    rtos_osal_critical_exit(state);

    /* Close the app */
    _Exit(0);
}

void xscope_fileio_tasks_create(unsigned priority, void* app_data) {
    xTaskCreate((TaskFunction_t)xscope_fileio_task,
                "xscope_fileio",
                RTOS_THREAD_STACK_SIZE(xscope_fileio_task),
                app_data,
                priority,
                &xscope_fileio_task_handle);
    
    // Define the core affinity mask such that this task can only run on a specific core
    UBaseType_t uxCoreAffinityMask = 0x10;

    /* Set the core affinity mask for the task. */
    vTaskCoreAffinitySet( xscope_fileio_task_handle, uxCoreAffinityMask );                
}
