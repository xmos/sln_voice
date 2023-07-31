// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#ifndef _SRC_POLY_VPU_H_
#define _SRC_POLY_VPU_H_

#include "stdint.h"
#include "src_low_level.h"

// If you want to undestand any of this better,
// read Richard G. Lyons "Understanding digital signal processing"
// capters 10.1 - 10.7, 10.10 - 10.11

/**
 * @brief Performs VPU-optimised 72 taps polyphase fixed-factor-of-3 downsampling
 * 
 * @param samp_in       Values to be downsampled
 * @param samp_out      Downsampled output
 * @param coefs_ff3     Three-phase FIR coefficients array with [3][24] dimentions
 * @param state_ds      Three-phase FIR state array with [3][24] dimentions
 */
static inline void src_ff3_72t_ds(int32_t samp_in[3], int32_t samp_out[1], const int32_t coefs_ff3[3][24], int32_t state_ds[3][24])
{
    int64_t acc = 0;
    acc += fir_s32_24t(state_ds[0], coefs_ff3[0], samp_in[0]);
    acc += fir_s32_24t(state_ds[1], coefs_ff3[1], samp_in[1]);
    acc += fir_s32_24t(state_ds[2], coefs_ff3[2], samp_in[2]);

    samp_out[0] = (int32_t)acc;
}

/**
 * @brief Performs VPU-optimised 72 taps polyphase fixed-factor-of-3 upsampling
 * 
 * @param samp_in       Value to be upsampled
 * @param samp_out      Upsampled output
 * @param coefs_ff3     Three-phase FIR coefficients array with [3][24] dimentions
 * @param state_ds      FIR state array with 24 elements in it
 */
static inline void src_ff3_72t_us(int32_t samp_in[1], int32_t samp_out[3], const int32_t coefs_ff3[3][24], int32_t state_us[24])
{
    samp_out[0] = fir_s32_24t(state_us, coefs_ff3[2], samp_in[0]) * 3;
    samp_out[1] = conv_s32_24t(state_us, coefs_ff3[1]) * 3;
    samp_out[2] = conv_s32_24t(state_us, coefs_ff3[0]) * 3;
}

/**
 * @brief Performs VPU-optimised 96 taps polyphase fixed-factor-of-3 downsampling
 * 
 * @param samp_in       Values to be downsampled
 * @param samp_out      Downsampled output
 * @param coefs_ff3     Three-phase FIR coefficients array with [3][32] dimentions
 * @param state_ds      Three-phase FIR state array with [3][32] dimentions
 */
static inline void src_ff3_96t_ds(int32_t samp_in[3], int32_t samp_out[1], const int32_t coefs_ff3[3][32], int32_t state_ds[3][32])
{
    int64_t acc = 0;
    acc += fir_s32_32t(state_ds[0], coefs_ff3[0], samp_in[0]);
    acc += fir_s32_32t(state_ds[1], coefs_ff3[1], samp_in[1]);
    acc += fir_s32_32t(state_ds[2], coefs_ff3[2], samp_in[2]);

    samp_out[0] = (int32_t)acc;
}

/**
 * @brief Performs VPU-optimised 96 taps polyphase fixed-factor-of-3 upsampling
 * 
 * @param samp_in       Value to be upsampled
 * @param samp_out      Upsampled output
 * @param coefs_ff3     Three-phase FIR coefficients array with [3][32] dimentions
 * @param state_ds      FIR state array with 32 elements in it
 */
static inline void src_ff3_96t_us(int32_t samp_in[1], int32_t samp_out[3], const int32_t coefs_ff3[3][32], int32_t state_us[32])
{
    samp_out[0] = fir_s32_32t(state_us, coefs_ff3[2], samp_in[0]) * 3;
    samp_out[1] = conv_s32_32t(state_us, coefs_ff3[1]) * 3;
    samp_out[2] = conv_s32_32t(state_us, coefs_ff3[0]) * 3;
}

/**
 * @brief Performs VPU-optimised 96 taps polyphase rational factor 2/3 downsampling
 * 
 * @param samp_in       Values to be downsampled
 * @param samp_out      Downsampled output
 * @param coefs_ff3     Two-phase FIR coefficients array with [2][48] dimentions
 * @param state_ds      FIR state array with 48 lements in it
 */
static inline void src_rat_2_3_96t_ds(int32_t samp_in[3], int32_t samp_out[2], const int32_t coefs_ds[2][48], int32_t state_ds[48])
{
    push_s32_48t(state_ds, samp_in[0]);
    samp_out[0] = fir_s32_48t(state_ds, coefs_ds[0], samp_in[1]) * 2;
    samp_out[1] = fir_s32_48t(state_ds, coefs_ds[1], samp_in[2]) * 2;
}

/**
 * @brief Performs VPU-optimised 96 taps polyphase rational factor 3/2 upsampling
 * 
 * @param samp_in       Values to be upsampled
 * @param samp_out      Upsampled output
 * @param coefs_ff3     Three-phase FIR coefficients array with [3][32] dimentions
 * @param state_ds      FIR state array with 32 lements in it
 */
static inline void src_rat_3_2_96t_us(int32_t samp_in[2], int32_t samp_out[3], const int32_t coefs_us[3][32], int32_t state_us[32])
{
    samp_out[0] = fir_s32_32t(state_us, coefs_us[0], samp_in[0]) * 3;
    samp_out[1] = conv_s32_32t(state_us, coefs_us[2]) * 3;
    samp_out[2] = fir_s32_32t(state_us, coefs_us[1], samp_in[1]) * 3;
}

#endif // _SRC_POLY_VPU_H_
