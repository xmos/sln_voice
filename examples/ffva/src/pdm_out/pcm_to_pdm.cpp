// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <stdint.h>
#include <assert.h>

#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"

#include "app_conf.h"
#include "platform/driver_instances.h"

#include <xcore/channel_streaming.h>
#include <xcore/interrupt.h>
#include <xcore/thread.h>

#include "pcm_to_pdm.h"
#include "rtos_printf.h"

#include "upsample.h"
#include "delta_sigma.h"
#include "mic_array/frame_transfer.h"
#include "mic_array/etc/filters_default.h"

#define SAMPLES_PER_FRAME   MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME //1
#define MIC_COUNT           MIC_ARRAY_CONFIG_MIC_COUNT //1

extern chanend_t c_audio_frames;

static StreamBufferHandle_t pcm_stream = NULL;
#define ASR_CHANNEL             (0)
#define COMMS_CHANNEL           (1)
void upsample_x192_thread(
    chanend_t c_sample_in,
    chanend_t c_sample_out)
{
  int32_t state_x8[32] = {0};
  int32_t state_x24[8] = {0};

  int32_t sample[MIC_COUNT];
  int32_t samples_x8[8];
  int32_t samples_x24[24];

  while(1){
    ma_frame_rx(sample, c_sample_in, MIC_COUNT, SAMPLES_PER_FRAME);
    upsample_x8(samples_x8, sample[0], upsample_x8_coef, state_x8);

    for(int n = 0; n < 8; n++){
      upsample_x24(samples_x24, samples_x8[n], upsample_x24_coef, state_x24);

      for(int k = 0; k < 24; k++)
        s_chan_out_word(c_sample_out, samples_x24[k]);
    }
  }
}

void pcm_to_pdm(chanend_t c_sample_in,
                chanend_t c_sample_out)
{
  float state_ds3[4] = DELTA_SIGMA3_INITIAL_STATE;
  float samples_in[32] = {0};

  while(1){

    chan_in_to_floats(samples_in, c_sample_in, -30);

    uint32_t pdm_data = delta_sigma3_float(samples_in,
                                           state_ds3,
                                           delta_sigma3_params);

    for(int k = 0; k < 3; k++){
      assert(state_ds3[k] >= -10);
      assert(state_ds3[k] <=  10);
    }

    s_chan_out_word(c_sample_out, pdm_data);
  }
}

/**
 * 
 */
streaming_channel_t s_audio_data;
streaming_channel_t c_ds3;
extern chanend_t c_audio_frames;
chanend_t c_pcm_in;
extern chanend_t c_pcm_out;

uint32_t pdm_data[PDM_DATA_SIZE];  // pdm data size for 1 frame (240 samples)
uint32_t *p_pdm_data_head = &(pdm_data[0]);
uint32_t *p_pdm_data_tail = &(pdm_data[PDM_DATA_SIZE-1]);
uint32_t *p_pdm_data_write;
extern uint32_t *p_pdm_data_read;

void pcm_to_pdm_full(
    chanend_t c_sample_in,
    chanend_t c_sample_out)
{
uint32_t buf[480];
  int32_t state_x8[32] = {0};
  int32_t state_x24[8] = {0};
  float state_ds3[4] = DELTA_SIGMA3_INITIAL_STATE;

  int32_t sample[MIC_COUNT];
  int32_t samples_x8[8];
  int32_t samples_x24[96];

  float samp_float[32] = {0};

  uint32_t pdm_block;

  int32_t i, t0=0x0, t1=1, t2=1, dat_cnt=2;

  c_pcm_in = chanend_alloc();
  c_ds3 = s_chan_alloc();
  s_audio_data = s_chan_alloc();
  

//  local_thread_mode_set_bits((thread_mode_t) (thread_mode_fast | thread_mode_high_priority));

  rtos_printf("pcm to pdm full on tile %d core %d mode %lx\n", THIS_XCORE_TILE, rtos_core_id_get(), local_thread_mode_get_bits());
  memset(&(pdm_data[0]), 0x00, PDM_DATA_SIZE);
  p_pdm_data_write = p_pdm_data_head;

  while(1){
    // NOTE: if PCM samples are coming from the mic array, ma_frame_rx() should
    //       be used because it does a channel transaction. Otherwise this
    //       can be replaced with basic channel input
    //ma_frame_rx(sample, s_audio_data.end_b, MIC_COUNT, SAMPLES_PER_FRAME);
    //ma_frame_rx(sample, s_audio_data.end_b, 2, 30);
    
    s_chan_in_buf_word(s_audio_data.end_b, (uint32_t *) &(buf[0]), (size_t) appconfAUDIO_PIPELINE_FRAME_ADVANCE * 2);
    for (i = 0; i < appconfAUDIO_PIPELINE_FRAME_ADVANCE; i++) {

      // only works in Q28[-0.5, +0.5] range
      if (buf[i] & 0x80000000) {
        sample[0] = ((buf[i]&0x7fffffff) >> 3) | 0xf0000001;
      } else {
        sample[0] = (buf[i] >> 3);
      } 
      
      upsample_x8(samples_x8, sample[0], upsample_x8_coef, state_x8);

      for(int k = 0; k < 2; k++){

        upsample_x24(&samples_x24[ 0], samples_x8[4*k+0], 
                     upsample_x24_coef, state_x24);
        // +24 -> 24
        upsample_x24(&samples_x24[24], samples_x8[4*k+1], 
                     upsample_x24_coef, state_x24);
        // +24 -> 48

        vect_to_floats(samp_float, &samples_x24[ 0], -30, 32);
        *p_pdm_data_write = delta_sigma3_float(samp_float, state_ds3, delta_sigma3_params);
        p_pdm_data_write++;
        // -32 -> 16

        upsample_x24(&samples_x24[48], samples_x8[4*k+2], 
                     upsample_x24_coef, state_x24);
        // +24 -> 40

        vect_to_floats(samp_float, &samples_x24[32], -30, 32);
        *p_pdm_data_write = delta_sigma3_float(samp_float, state_ds3, delta_sigma3_params);
        p_pdm_data_write++;
        // -32 -> 8

        upsample_x24(&samples_x24[72], samples_x8[4*k+3], 
                     upsample_x24_coef, state_x24);
        // +24 -> 32

        vect_to_floats(samp_float, &samples_x24[64], -30, 32);
        *p_pdm_data_write = delta_sigma3_float(samp_float, state_ds3, delta_sigma3_params);
        p_pdm_data_write++;
        // -32 -> 0
      }
    }

    if (p_pdm_data_write > p_pdm_data_tail) {
      p_pdm_data_write = p_pdm_data_head;
    }
    if (dat_cnt == 0) {
      s_chan_out_word(c_ds3.end_a, t0);
    } else {
      dat_cnt--;
    }
  }
}

using TDecimator = mic_array::TwoStageDecimator<MIC_COUNT, 
                                                STAGE2_DEC_FACTOR, 
                                                STAGE2_TAP_COUNT>;

void pdm_to_pcm(chanend_t c_sample_in,
                chanend_t c_sample_out)
{
  TDecimator dec;
  dec.Init((uint32_t*) stage1_coef, stage2_coef, stage2_shr);

  while(1){

    uint32_t pdm_data[6];
    int32_t frame[MIC_COUNT];

    for(int k = 0; k < 6; k++)
      pdm_data[k] = s_chan_in_word(c_sample_in);

    dec.ProcessBlock(frame, pdm_data);

    frame[0] = frame[0] << 3;
    ma_frame_tx(c_sample_out, frame, MIC_COUNT, SAMPLES_PER_FRAME);
  }
}

void pcm2pdm(int32_t sample[])
{
  int32_t state_x8[32] = {0};
  int32_t state_x24[8] = {0};
  float state_ds3[4] = DELTA_SIGMA3_INITIAL_STATE;

  //int32_t sample[MIC_COUNT];
  int32_t samples_x8[8];
  int32_t samples_x24[96];

  float samp_float[32] = {0};

  uint32_t pdm_block;
  uint32_t pdm_data[6];
  int32_t frame[2];

  TDecimator dec;
  dec.Init((uint32_t*) stage1_coef, stage2_coef, stage2_shr);

    // NOTE: if PCM samples are coming from the mic array, ma_frame_rx() should
    //       be used because it does a channel transaction. Otherwise this
    //       can be replaced with basic channel input
    //ma_frame_rx(sample, c_sample_in, MIC_COUNT, SAMPLES_PER_FRAME);
    rtos_printf("s_in: %lx     %lx\n", sample[0], sample[1]);
    //int32_t tmpn = sample[0];
    upsample_x8(samples_x8, sample[0], upsample_x8_coef, state_x8);
    //upsample_x8(samples_x8, tmpn, upsample_x8_coef, state_x8);

    for(int k = 0; k < 2; k++){

      upsample_x24(&samples_x24[ 0], samples_x8[4*k+0], 
                   upsample_x24_coef, state_x24);
      // +24 -> 24
      upsample_x24(&samples_x24[24], samples_x8[4*k+1], 
                   upsample_x24_coef, state_x24);
      // +24 -> 48
      
      vect_to_floats(samp_float, &samples_x24[ 0], -30, 32);
      pdm_block = delta_sigma3_float(samp_float, state_ds3, 
                                     delta_sigma3_params);
      //s_chan_out_word(c_ds3, pdm_block);
      //rtos_printf("pb0: %lx\n", pdm_block); pdm_block=0;
      pdm_data[k*2] = pdm_block;
      // -32 -> 16

      upsample_x24(&samples_x24[48], samples_x8[4*k+2], 
                   upsample_x24_coef, state_x24);
      // +24 -> 40

      vect_to_floats(samp_float, &samples_x24[32], -30, 32);
      pdm_block = delta_sigma3_float(samp_float, state_ds3, 
                                     delta_sigma3_params);
      //s_chan_out_word(c_ds3, pdm_block);
      //rtos_printf("pb1: %lx\n", pdm_block);pdm_block=0;
      pdm_data[k*2+1] = pdm_block;
      // -32 -> 8

      upsample_x24(&samples_x24[72], samples_x8[4*k+3], 
                   upsample_x24_coef, state_x24);
      // +24 -> 32

      vect_to_floats(samp_float, &samples_x24[64], -30, 32);
      pdm_block = delta_sigma3_float(samp_float, state_ds3, 
                                     delta_sigma3_params);
      //s_chan_out_word(c_ds3, pdm_block);
      //rtos_printf("pb2: %lx\n", pdm_block);
      pdm_data[k*2+2] = pdm_block;
      // -32 -> 0
  }
    dec.ProcessBlock(frame, pdm_data);
    frame[0] = frame[0] << 3;
    rtos_printf("frame: %lx     %lx\n", frame[0], frame[1]);
}
