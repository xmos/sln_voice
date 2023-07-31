// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#ifndef _SRC_LOW_LEVEL_H_
#define _SRC_LOW_LEVEL_H_

/**
 * @brief Perfoms VPU-optimised convolution for s32 type integers
 * 
 * @param samples   Samples array
 * @param coef      FIR coefficients array
 * @note Both samples and coef has to have 24 values int32_t in them
 * @note Both samples and coef have to be 8 bit aligned
 */
int32_t conv_s32_24t(const int32_t * samples, const int32_t * coef);

/**
 * @brief Perforns VPU-optimised FIR filtering for s32 type integers
 * 
 * @param state     State that keep previous samples
 * @param coef      FIR coefficients array
 * @param new_samp  New sample to put in the state
 * @note Both state and coef has to have 24 values int32_t in them
 * @note Both state and coef have to be 8 bit aligned
 */
int32_t fir_s32_24t(int32_t * state, const int32_t * coef, int32_t new_samp);

/**
 * @brief Perfoms VPU-optimised convolution for s32 type integers
 * 
 * @param samples   Samples array
 * @param coef      FIR coefficients array
 * @note Both samples and coef has to have 32 values int32_t in them
 * @note Both samples and coef have to be 8 bit aligned
 */
int32_t conv_s32_32t(const int32_t * samples, const int32_t * coef);

/**
 * @brief Perforns VPU-optimised FIR filtering for s32 type integers
 * 
 * @param state     State that keep previous samples
 * @param coef      FIR coefficients array
 * @param new_samp  New sample to put in the state
 * @note Both state and coef has to have 32 values int32_t in them
 * @note Both state and coef have to be 8 bit aligned
 */
int32_t fir_s32_32t(int32_t * state, const int32_t * coef, int32_t new_samp);

/**
 * @brief Perforns VPU-optimised ring buffer shift for s32 type integers
 * 
 * @param state     State that keep previous samples
 * @param new_samp  New sample to put in the state
 * @note Both state and coef has to have 48 values int32_t in them
 * @note Both state and coef have to be 8 bit aligned
 */
void push_s32_48t(int32_t * state, int32_t new_samp);

/**
 * @brief Perforns VPU-optimised FIR filtering for s32 type integers
 * 
 * @param state     State that keep previous samples
 * @param coef      FIR coefficients array
 * @param new_samp  New sample to put in the state
 * @note Both state and coef has to have 48 values int32_t in them
 * @note Both state and coef have to be 8 bit aligned
 */
int32_t fir_s32_48t(int32_t * state, const int32_t * coef, int32_t new_samp);

#endif // _SRC_LOW_LEVEL_H_
