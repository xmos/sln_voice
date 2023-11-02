// Copyright 2021-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef __dbcalc_h__
#define __dbcalc_h__

#include <stdint.h>

/* Function: db_to_mult

     This function converts decibels into a volume multiplier. It uses a fixed-point polynomial approximation
     to 10^(db/10).

   Parameters:
       db               - The db value to convert.
       db_frac_bits     - The number of binary fractional bits in the supplied decibel value
       result_frac_bits - The number of required fractional bits in the result.

   Returns:
       The multiplier value as a fixed point value with the number of fractional bits as specified by
       the result_frac_bits parameter.
*/
uint32_t db_to_mult(int32_t db, int32_t db_frac_bits, int32_t result_frac_bits);

#endif // __dbcalc_h__
