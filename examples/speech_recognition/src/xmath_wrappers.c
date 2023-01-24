// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <stdint.h>
#include <string.h>

#include "xmath_wrappers.h"

#include "app_conf.h"
#include "flash.h"
#include "xmath/xmath.h"

#define ROUND_UPTO_NEAREST_MULTIPLE(VAL, MULT) ((VAL + MULT) & ~31)

#define MAX_N_COLS                 (540)
#define MAX_MATRIX_SIZE_BYTES      (10000)
#define MAX_SCRATCH_SIZE_BYTES     ROUND_UPTO_NEAREST_MULTIPLE(MAX_N_COLS, 16)

__attribute__((aligned(4))) 
static int8_t matrix_scratch[MAX_MATRIX_SIZE_BYTES] = {0};

__attribute__((aligned(4)))
static int8_t mat_mul_scratch[MAX_SCRATCH_SIZE_BYTES] = {0};

// This function wraps a call to lib_xcore_math's mat_mul_s8_x_s16_yield_s32 function.
// Prior to calling mat_mul_s8_x_s16_yield_s32, it loads the matrix coeffs from flash 
// into a scratch buffer.  This costs some SRAM but allows for VPU acceleration and is 
// MUCH faster.  You do not need to call this if the matrix is already in SRAM.
//
// This function implementation would be provided by XMOS.  This allows us to provide 2
// implementations of it - one for when the NET is in SRAM, another for when the NET is in flash.
// SensoryLib would call the version provided by the application.  By default, SensoryLib 
// would provide a "weak" symbol that would pass-through to the mat_mul_s8_x_s16_yield_s32
// function without reading from flash.  
//
// NOTE: Be sure to use the correct MAX_MATRIX_SIZE_BYTES at the beginning of this source file

// __attribute__((weak))
// void mat_mul(int32_t *output, const int8_t *matrix, const int16_t *input_vector,
//             const signed int M_rows, const signed int N_cols) {
//   // just pass-through to vectorized mat_mul
//   mat_mul_s8_x_s16_yield_s32(output, matrix, input_vector, M_rows,
//                                  N_cols, &mat_mul_scratch[0]);
// }

void mat_mul(int32_t *output, const int8_t *matrix, const int16_t *input_vector,
            const signed int M_rows, const signed int N_cols) {

  size_t matrix_size_bytes = M_rows * N_cols;

  // read matrix into scratch
  flash_read_wrapper((unsigned)matrix, (unsigned *)&matrix_scratch[0], matrix_size_bytes>>2);
  // then call vectorized mat_mul
  mat_mul_s8_x_s16_yield_s32(output, matrix_scratch, input_vector, M_rows,
                                 N_cols, &mat_mul_scratch[0]);
};

void test_mat_mul() {
    signed int M_rows = 24;
    signed int N_cols = 16;

    int8_t *matrix = (int8_t *)0x40000000;

    int16_t vector[16] = {0};

    int32_t output[16] = {0};

    mat_mul(output, matrix, vector, M_rows, N_cols);
}