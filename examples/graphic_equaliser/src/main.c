// CopyriGTH 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <platform.h>
#include <xcore/parallel.h>
#include <xcore/channel.h>
#include <xcore/chanend.h>
#include <xcore/select.h>
#include <xscope.h>
#include <stdlib.h>
#include <string.h>
#include "equaliser/equaliser.h"

#define BYTES_FROM_XSCOPE 256
#define FRAME_LENGTH 240
#define INT_SIZE sizeof(int32_t)

void rx(chanend_t xscope_chan, chanend_t read_chan){
  char buffer[BYTES_FROM_XSCOPE];
  int32_t data[FRAME_LENGTH];
  int bytes_read = 0;
  int input_bytes = 0;
  xscope_mode_lossless();
  xscope_connect_data_from_host(xscope_chan);
  while(1){
    SELECT_RES(CASE_THEN(xscope_chan, read_host_data)){
      read_host_data:{
        xscope_data_from_host(xscope_chan, &buffer[0], &bytes_read);
        //  we can only recieve 256 bytes from the xscope channel,
        //  so we need to form a circular buffer here
        memcpy(&data[0] + input_bytes / INT_SIZE, &buffer[0], bytes_read);
        input_bytes += bytes_read;
        if(input_bytes / INT_SIZE == FRAME_LENGTH){
          //  send 15 ms frame to equalise
          chan_out_buf_word(read_chan, (uint32_t *)data, FRAME_LENGTH);
          input_bytes = 0;
        }
      }
    }
  }
}

void equalise(chanend_t read_chan, chanend_t write_chan){
  int32_t DWORD_ALIGNED buffer[FRAME_LENGTH];
  equaliser_t eq;
  eq_init(&eq);
  //  set the band gains here before calculating biquads coefficiens
  //  this example does 300 Hz low-pass
  //  you can find the center frequencies table in equaliser.c
  for(int i = 14; i < NUM_BANDS; i++){
    eq.band_dBgain[i] = -10.0;
    }
  eq_get_biquads(&eq);
  while(1){
    chan_in_buf_word(read_chan, (uint32_t *)buffer, FRAME_LENGTH);
    eq_process_frame(&eq, buffer);
    chan_out_buf_word(write_chan, (uint32_t *)buffer, FRAME_LENGTH);
  }
}

void tx(chanend_t write_chan){
  int32_t buffer[FRAME_LENGTH];
  while(1){
    chan_in_buf_word(write_chan, (uint32_t *)buffer, FRAME_LENGTH);
    for(int v = 0; v < FRAME_LENGTH; v++){
      xscope_int(0, buffer[v]);
    }
  }
}
