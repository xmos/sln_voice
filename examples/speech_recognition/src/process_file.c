// Copyright 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <xcore/hwtimer.h>

#include "app_conf.h"
#include "asr.h"
#include "device_memory_impl.h"
#include "wav_utils.h"
#include "xscope_io_device.h"

// Include the example port model data
#include "asr_example/asr_example_model.h"

#define MAX_CHANNELS         (1)
#define BRICK_SIZE_SAMPLES   (240) 
#define BRICK_SIZE_BYTES     (BRICK_SIZE_SAMPLES*sizeof(int16_t))

const char* word_id2text(int word_id) {
    switch (word_id) {
        case 100:
            return "Hello XMOS";
            break;
        default:
            return "Unknown";
            break; 
    }
}

void process_file() {
    xscope_file_t file;
    wav_header header_struct;
    size_t header_size;
    size_t brick_count;        
    int16_t brick[BRICK_SIZE_SAMPLES];

    uint32_t timer_start = 0;
    uint32_t timer_end = 0;
    uint32_t timer_duration = 0;
    uint32_t total_duration = 0;
    uint32_t max_duration = 0;
    uint32_t avg_duration = 0;

    devmem_manager_t devmem_mgr;
    asr_port_t asr_port = NULL;
    asr_error_t asr_error;
    asr_result_t asr_result;

    printf("Opening %s\n", appconfINPUT_FILENAME);

    // Initialize the device memory manager
    devmem_init(&devmem_mgr);

    // TODO: Pass your model data here (and grammar data here if required)
    asr_port = asr_init((int32_t *) model_data, (int32_t *) grammar_data, &devmem_mgr);

    printf("Opening %s\n", appconfINPUT_FILENAME);

    file = xscope_open_file(appconfINPUT_FILENAME, "rb");

    // Validate input wav file
    if(get_wav_header_details(&file, &header_struct, &header_size) != 0){
        printf("Error: error in get_wav_header_details()\n");
        _Exit(1);
    }
    assert(header_struct.bit_depth == 16);
    assert(header_struct.num_channels == MAX_CHANNELS);

    xscope_fseek(&file, header_size, SEEK_SET);

    // Calculate number of bricks in the wav file
    brick_count = wav_get_num_frames(&header_struct) / BRICK_SIZE_SAMPLES;

    printf("Processing %d bricks\n", brick_count);

    for(unsigned b=0; b<brick_count; b++) {
        if ((b > 0)  && (b % 100 == 0))
            printf("Processing brick %d\n", b);

        memset((uint8_t *)&brick[0], 0, BRICK_SIZE_BYTES);
        xscope_fseek(&file, wav_get_frame_start(&header_struct, b * BRICK_SIZE_SAMPLES, header_size), SEEK_SET);
        xscope_fread(&file, (uint8_t *)&brick[0], BRICK_SIZE_BYTES);

        // Process the audio samples, timing duration of the call to asr_process
        timer_start = get_reference_time();
        asr_error = asr_process(asr_port, brick, BRICK_SIZE_SAMPLES);
        timer_end = get_reference_time();
        //printf("Duration: %lu (us)\n", timer_duration);

        timer_duration = (timer_end - timer_start) / 100;
        total_duration += timer_duration;
        if (timer_duration > max_duration)
            max_duration = timer_duration;

        if (asr_error == ASR_OK) {
            asr_error = asr_get_result(asr_port, &asr_result);
            if ((asr_error == ASR_OK) && (asr_result.id > 0)) {
                printf("Keyword: %s\n", word_id2text(asr_result.id));
            }
        }

    }
    
    avg_duration = total_duration / brick_count;

    printf("Max duration: %lu (us)\n", max_duration);
    printf("Avg duration: %lu (us)\n", avg_duration);

    if (avg_duration > 15000) {
       printf("WARNING: Avg duration exceeds 15 (ms)\n");
    }

    asr_release(asr_port);
    xscope_close_all_files();
    _Exit(0);
}
