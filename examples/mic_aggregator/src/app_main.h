// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#pragma once

#include <stdint.h>
#include "app_config.h"

#define NUM_AUDIO_BUFFERS   3

typedef struct audio_frame_t{
    int32_t data[MIC_ARRAY_CONFIG_MIC_COUNT][MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME];
} audio_frame_t;

// Macro to adjust input pad timing for the round trip delay. Supports 0 (default) to 5 core clock cycles.
// Larger numbers increase hold time but reduce setup time.
#define PORT_DELAY      0x7007
#define DELAY_SHIFT     0x3
#define set_pad_delay(port, delay)  {__asm__ __volatile__ ("setc res[%0], %1": : "r" (port) , "r" ((delay << 0x3) | PORT_DELAY));}

// Macro to adjust input pad capture clock edge
#define PORT_SAMPLE     0x4007
#define SAMPLE_SHIFT    0x3
#define SAMPLE_RISING   0x0 // Default for input port. Reduces setup time, increases hold time.
#define SAMPLE_FALLING  0x1 // Increases setup time, reduces hold time
#define set_pad_sample_edge(port, edge)  {__asm__ __volatile__ ("setc res[%0], %1": : "r" (port) , "r" ((edge << 0x3) | PORT_SAMPLE));}


// Pad control defines
#define PAD_CONTROL     0x0006
#define DRIVE_2MA       0x0
#define DRIVE_4MA       0x1
#define DRIVE_8MA       0x2
#define DRIVE_12MA      0x3
#define DRIVE_SHIFT     20
#define ENABLE_SCHMITT  (1 << 23)

// Macro to adjust the pad output drive strength
#define set_pad_drive_strength(port, strength)  {__asm__ __volatile__ ("setc res[%0], %1": : "r" (port) , "r" ((strength << DRIVE_SHIFT) | PAD_CONTROL));}
// Macro to enable the schmitt input
#define set_pad_input_schmitt(port)  {__asm__ __volatile__ ("setc res[%0], %1": : "r" (port) , "r" (ENABLE_SCHMITT | PAD_CONTROL));}


// Macros to convert between two bytes and U16
#define LOWER_BYTE_FROM_U16(u16)                ((uint8_t)(u16 & 0xff))
#define UPPER_BYTE_FROM_U16(u16)                ((uint8_t)(u16 >> 8))
#define U16_FROM_BYTES(upper_byte, lower_byte)  (((uint16_t)upper_byte << 8) | (uint16_t)lower_byte)
