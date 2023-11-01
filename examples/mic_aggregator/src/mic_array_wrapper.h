// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#pragma once

#include <stdint.h>

#include "app_config.h"
#include "app_common.h"
#include "mic_array.h"
#include "util/audio_buffer.h"

typedef struct app_ctx {
  audio_ring_buffer_t rb; // Ring buffer for the mic array audio samples.
#if (MIC_ARRAY_TILE == 0 || USE_BUTTONS)
  chanend_t c_intertile; // Used to transfer mic array samples on one tile to an I2S DAC on the other tile.
#endif
} app_context_t;

C_API_START

MA_C_API
void app_mic_array_init( void );

MA_C_API
void app_mic_array_task( chanend_t c_frames_out );

MA_C_API
void app_mic_array_assertion_enable( void );

MA_C_API
void app_mic_array_assertion_disable( void );

MA_C_API
void app_pdm_rx_task( void );

C_API_END
