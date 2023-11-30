// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#if !X86_BUILD
    #include <platform.h>
    #include <xs1.h>
    #include <xcore/assert.h>
#else
    #include <assert.h>
    #define xassert assert
#endif
#include <xmath/xmath.h>
#include "pseudo_rand.h"
#include "div.h"

void test_float_div(unsigned seed, bool verbose)
{

    for(int itt=0; itt<(1<<8); itt++)
    {
        int32_t dividend_mant = pseudo_rand_uint32(&seed);
        int32_t dividend_exp =  pseudo_rand_int(&seed, -16, 16);
        float_s32_t dividend = {dividend_mant, dividend_exp};

        int32_t divisor_mant = pseudo_rand_uint32(&seed);
        int32_t divisor_exp =  pseudo_rand_int(&seed, -16, 16);
        float_s32_t divisor = {divisor_mant, divisor_exp};

        double f_dividend = ldexp((unsigned)dividend_mant, dividend_exp);
        double f_divisor = ldexp((unsigned)divisor_mant, divisor_exp);

        // reference
        double ref = f_dividend / f_divisor;

        // float_div()
        float_s32_t res = float_div(dividend, divisor);
        double dut = ldexp((unsigned)res.mant, res.exp);

        double abs_diff = fabs(ref - dut);
        double rel_error = fabs(abs_diff/(ref + ldexp(1, -40)));
        double thresh = ldexp(1, -31);

        if(verbose)
        {
            printf("float_div: itt %d: dut = %.15f. ref = %.15f, rel_error = %.15f, thresh = %.15f \n", itt, dut, ref, rel_error, thresh);
        }

        if(rel_error > thresh)
        {
            printf("FAIL, test_float_div(): itt %d: dut = %.15f. ref = %.15f, rel_error = %.15f, thresh = %.15f\n", itt, dut, ref, rel_error, thresh);
            xassert(0);
        }
    }
}

void test_div_fixed_output_q_format(unsigned seed, bool verbose)
{

    for(int itt=0; itt<(1<<8); itt++)
    {
        int32_t dividend_mant = pseudo_rand_uint32(&seed);
        int32_t dividend_exp =  pseudo_rand_int(&seed, -16, 16);
        float_s32_t dividend = {dividend_mant, dividend_exp};

        int32_t divisor_mant = pseudo_rand_uint32(&seed);
        int32_t divisor_exp =  pseudo_rand_int(&seed, -16, 16);
        float_s32_t divisor = {divisor_mant, divisor_exp};

        double f_dividend = ldexp((unsigned)dividend_mant, dividend_exp);
        double f_divisor = ldexp((unsigned)divisor_mant, divisor_exp);

        // reference
        double ref = f_dividend / f_divisor;
        float_s32_t float_s32_ref = f64_to_float_s32(ref);

        // float_div_u64_fixed_output_q_format()
        int32_t output_q_format = -float_s32_ref.exp; // To ensure that the division output fits in the given output format
        uint64_t res = float_div_u64_fixed_output_q_format(dividend, divisor, output_q_format);
        double dut = ldexp(res, -output_q_format);

        double abs_diff = fabs(ref - dut);
        double rel_error = fabs(abs_diff/(ref + ldexp(1, -40)));
        double thresh = ldexp(1, -31);

        if(verbose)
        {
            printf("float_div_u64_fixed_output_q_format: itt %d: res = %.15f. ref = %.15f, rel_error = %.15f, thresh = %.15f \n", itt, dut, ref, rel_error, thresh);
        }

        if(rel_error > thresh)
        {
            printf("FAIL, test_div_fixed_output_q_format(): itt %d: dut = %.15f. ref = %.15f, rel_error = %.15f, thresh = %.15f\n", itt, dut, ref, rel_error, thresh);
            xassert(0);
        }
    }
}


int main(int argc, char *argv[])
{
    unsigned seed = 123450;

    bool verbose = false;

    test_float_div(seed, verbose);

    test_div_fixed_output_q_format(seed, verbose);


}
