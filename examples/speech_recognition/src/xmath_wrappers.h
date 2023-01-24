// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef MATH_WRAPPERS_H_
#define MATH_WRAPPERS_H_

void mat_mul(int32_t *output, const int8_t *matrix, const int16_t *input_vector,
            const signed int M_rows, const signed int N_cols);

void test_mat_mul();

#endif /* MATH_WRAPPERS_H_ */