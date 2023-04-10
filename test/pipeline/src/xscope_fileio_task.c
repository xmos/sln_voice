// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#include <platform.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "soc_xscope_host.h"

#include "app_conf.h"
#include "platform/driver_instances.h"
#include "xscope_fileio_task.h"
#include "xscope_io_device.h"
#include "wav_utils.h"

#ifndef DWORD_ALIGNED
#define DWORD_ALIGNED     __attribute__ ((aligned(8)))
#endif

static TaskHandle_t xscope_fileio_task_handle;
static QueueHandle_t tx_to_host_queue;

static xscope_file_t infile;
static xscope_file_t outfile;

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

size_t xscope_fileio_tx_to_host(uint8_t *buf, size_t size_bytes) {
    size_t ret = 0;
    xQueueSend(tx_to_host_queue, buf, portMAX_DELAY);

    return ret;
}

size_t xscope_fileio_rx_from_host(void *input_app_data, int8_t **input_data_frame, size_t size_bytes) {

    size_t bytes_received = 0;
    bytes_received = rtos_intertile_rx_len(
            intertile_ctx,
            appconfXSCOPE_FILEIO_PORT,
            portMAX_DELAY);
    
    xassert(bytes_received == size_bytes);

    rtos_intertile_rx_data(
            intertile_ctx,
            input_data_frame,
            bytes_received);

    return bytes_received;
}

void xscope_fileio_user_done(void) {
    xTaskNotifyGive(xscope_fileio_task_handle);
}

/* This task reads the input file in chunks and sends it through the data pipeline
 * After reading the entire file, it will wait until the user has confirmed
 * all writing is complete before closing files.
 */
 /* NOTE:
  * xscope fileio uses events.  Only xscope_fread() currently, but wrapping
  * all calls in a critical section just in case */
void xscope_fileio_task(void *arg) {
    (void) arg;
    int state = 0;
    wav_header input_header_struct, output_header_struct;
    unsigned input_header_size;
    unsigned frame_count;
    unsigned brick_count;        
    uint32_t DWORD_ALIGNED in_buf_raw[appconfAUDIO_PIPELINE_FRAME_ADVANCE * appconfINPUT_CHANNELS];
    uint32_t DWORD_ALIGNED in_buf_int[appconfINPUT_CHANNELS * appconfAUDIO_PIPELINE_FRAME_ADVANCE];
    uint32_t DWORD_ALIGNED out_buf_raw[appconfOUTPUT_CHANNELS * appconfAUDIO_PIPELINE_FRAME_ADVANCE];
    uint32_t DWORD_ALIGNED out_buf_int[appconfAUDIO_PIPELINE_FRAME_ADVANCE * appconfOUTPUT_CHANNELS];
    size_t bytes_read = 0;

    /* Wait until xscope_fileio is initialized */
    while(xscope_fileio_is_initialized() == 0) {
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    tx_to_host_queue = xQueueCreate(1, appconfOUTPUT_BRICK_SIZE_BYTES);

    rtos_printf("Opening files\n");
    state = rtos_osal_critical_enter();
    {
        infile = xscope_open_file(appconfINPUT_FILENAME, "rb");
        outfile = xscope_open_file(appconfOUTPUT_FILENAME, "wb");
        // Validate input wav file
        if(get_wav_header_details(&infile, &input_header_struct, &input_header_size) != 0){
            rtos_printf("Error: error in get_wav_header_details()\n");
            _Exit(1);
        }
        xscope_fseek(&infile, input_header_size, SEEK_SET);
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
    brick_count = frame_count / appconfAUDIO_PIPELINE_FRAME_ADVANCE; 

    // Create output wav file
    rtos_printf("Writing output file header\n");
    wav_form_header(&output_header_struct,
        input_header_struct.audio_format,
        appconfOUTPUT_CHANNELS,
        input_header_struct.sample_rate,
        input_header_struct.bit_depth,
        brick_count*appconfAUDIO_PIPELINE_FRAME_ADVANCE);

    xscope_fwrite(&outfile, (uint8_t*)(&output_header_struct), WAV_HEADER_BYTES);

    // ensure the write above has time to complete before performing any reads
    vTaskDelay(pdMS_TO_TICKS(1000));

    rtos_printf("Processing %d bricks\n", brick_count);

    // Iterate over frame blocks and send the data to the first pipeline stage on tile[1]
    for(unsigned b=0; b<brick_count; b++) {
        memset(in_buf_raw, 0, appconfINPUT_BRICK_SIZE_BYTES);
        long input_location =  wav_get_frame_start(&input_header_struct, b * appconfAUDIO_PIPELINE_FRAME_ADVANCE, input_header_size);

        if (b % 100 == 0) {
            rtos_printf("Processing brick %d of %d\n", b, brick_count);
        }

        // Read from input wav file
        state = rtos_osal_critical_enter();
        {
            xscope_fseek(&infile, input_location, SEEK_SET);
            bytes_read = xscope_fread(&infile, (uint8_t *) &in_buf_raw[0], appconfINPUT_BRICK_SIZE_BYTES);
        }
        rtos_osal_critical_exit(state);

        // De-interleave input
        //  wav files are in frame-major order, pipeline expects sample-major order
        for(unsigned f=0; f<appconfAUDIO_PIPELINE_FRAME_ADVANCE; f++){
            for(unsigned ch=0; ch<appconfINPUT_CHANNELS; ch++) {
                in_buf_int[ch * appconfAUDIO_PIPELINE_FRAME_ADVANCE + f] = in_buf_raw[f * appconfINPUT_CHANNELS + ch];
            }
        }

        // Send audio to pipeline
        rtos_intertile_tx(intertile_ctx,
                        appconfXSCOPE_FILEIO_PORT,
                        &in_buf_int[0],
                        appconfINPUT_BRICK_SIZE_BYTES);

        // Read from tx_to_host queue 
        xQueueReceive(tx_to_host_queue, &out_buf_raw[0], portMAX_DELAY);

        // Interleaved output
        //  pipeline sample-major order, wav files are in frame-major order
        for (unsigned ch=0; ch<appconfOUTPUT_CHANNELS; ch++){
            for(unsigned f=0; f<appconfAUDIO_PIPELINE_FRAME_ADVANCE; f++) {
                out_buf_int[f * appconfOUTPUT_CHANNELS + ch] = out_buf_raw[ch * appconfAUDIO_PIPELINE_FRAME_ADVANCE + f];
            }
        }

        // Write brick to output wav file
        xscope_fwrite(&outfile, (uint8_t *) &out_buf_int[0], appconfOUTPUT_BRICK_SIZE_BYTES);
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
