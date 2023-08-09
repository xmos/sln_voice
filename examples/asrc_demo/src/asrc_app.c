
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <xcore/assert.h>
#include <xcore/chanend.h>
#include <xcore/channel.h>
#include "fileio.h"
#include "wav_utils.h"
#include "defines.h"

void fileio_task(const char *input_file_name, const char *output_file_name, chanend_t c_file_to_asrc, chanend_t c_asrc_to_file) {
    file_t input_file, output_file;
    // Open input wav file containing mic and ref channels of input data
    int ret = file_open(&input_file, input_file_name, "rb");
    if(ret)
    {
        printf("Failed to open input file\n");
        xassert(0);
    }
    // Open output wav file that will contain the AEC output
    ret = file_open(&output_file, output_file_name, "wb");
    if(ret)
    {
        printf("Failed to open output file\n");
        xassert(0);
    }
    wav_header input_header_struct, output_header_struct;
    unsigned input_header_size;
    if(get_wav_header_details(&input_file, &input_header_struct, &input_header_size) != 0){
        printf("error in get_wav_header_details()\n");
        _Exit(1);
    }
    
    file_seek(&input_file, input_header_size, SEEK_SET);
    // Ensure 32bit wav file
    if(input_header_struct.bit_depth != 32)
    {
        printf("Error: unsupported wav bit depth (%d) for %s file. Only 32 supported\n", input_header_struct.bit_depth, input_file_name);
        _Exit(1);
    }
    printf("SAMPLE_RATE = %d\n",input_header_struct.sample_rate);

    // Ensure input wav file contains correct number of channels 
    if(input_header_struct.num_channels != (NUM_CHANNELS)){
        printf("Error: wav num channels(%d) does not match %d\n", input_header_struct.num_channels, NUM_CHANNELS);
        _Exit(1);
    }

    unsigned frame_count = wav_get_num_frames(&input_header_struct);
    unsigned block_count = frame_count / INPUT_SAMPLES_PER_FRAME;

    printf("BLOCK count = %d\n",block_count);
    
    int num_test_frames = 10000;
    wav_form_header(&output_header_struct,
            input_header_struct.audio_format,
            NUM_CHANNELS,
            48000,
            32,
            num_test_frames*(INPUT_SAMPLES_PER_FRAME/4));

    file_write(&output_file, (uint8_t*)(&output_header_struct),  WAV_HEADER_BYTES);
    
    unsigned bytes_per_frame = wav_get_num_bytes_per_frame(&input_header_struct);
    printf("bytes_per_frame = %d\n",bytes_per_frame);

    int32_t input_read_buffer[INPUT_SAMPLES_PER_FRAME * NUM_CHANNELS] = {0}; // Array for storing interleaved input read from wav file
    int32_t output_buffer[INPUT_SAMPLES_PER_FRAME * NUM_CHANNELS] = {0}; // Array for storing interleaved input read from wav file
    uint32_t total_output_samples = 0;
    for(unsigned b=0;b<num_test_frames;b++)
    {
        long input_location =  wav_get_frame_start(&input_header_struct, b*INPUT_SAMPLES_PER_FRAME , input_header_size);
        file_seek (&input_file, input_location, SEEK_SET);
        file_read (&input_file, (uint8_t*)&input_read_buffer[0], bytes_per_frame* INPUT_SAMPLES_PER_FRAME );
        printf("Frame %d\n",b);
        chan_out_buf_word(c_file_to_asrc, (uint32_t*)input_read_buffer, (bytes_per_frame* INPUT_SAMPLES_PER_FRAME)/sizeof(int32_t));
        
        unsigned n_samps_out = chan_in_word(c_file_to_asrc);
        total_output_samples += n_samps_out;
        if(n_samps_out != 60)
        {
            printf("n_samps_out = %d\n", n_samps_out);
        }
        chan_in_buf_word(c_file_to_asrc, (uint32_t*)output_buffer, (n_samps_out * NUM_CHANNELS));

        file_write(&output_file, (uint8_t*)(output_buffer), output_header_struct.bit_depth/8 * n_samps_out * NUM_CHANNELS);
    }
    wav_form_header(&output_header_struct,
            input_header_struct.audio_format,
            NUM_CHANNELS,
            48000,
            32,
            total_output_samples);
    file_seek (&output_file, 0, SEEK_SET); // Rewrite the header
    file_write(&output_file, (uint8_t*)(&output_header_struct),  WAV_HEADER_BYTES);

    file_close(&input_file);
    file_close(&output_file);
    shutdown_session();
    exit(0);
}
