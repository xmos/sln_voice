// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include "div.h"
float_s32_t float_div(float_s32_t dividend, float_s32_t divisor)
{
    float_s32_t res;

    int dividend_hr;
    int divisor_hr;

 #if __xcore__
    asm( "clz %0, %1" : "=r"(dividend_hr) : "r"(dividend.mant) );
    asm( "clz %0, %1" : "=r"(divisor_hr) : "r"(divisor.mant) );
#else
    dividend_hr = __builtin_clz(dividend.mant);
    divisor_hr =  __builtin_clz(divisor.mant);
#endif


    int dividend_exp = dividend.exp - dividend_hr;
    int divisor_exp = divisor.exp - divisor_hr;


    uint64_t h_dividend = (uint64_t)((uint32_t)dividend.mant) << (dividend_hr);

    uint32_t h_divisor = ((uint32_t)divisor.mant) << (divisor_hr);

    uint32_t lhs = (h_dividend > h_divisor) ? 31 : 32;

    uint64_t normalised_dividend = h_dividend << lhs;

#if __xcore__
    uint32_t quotient = 0;
    uint32_t remainder = 0;
    uint32_t h = (uint32_t)(normalised_dividend>>32);
    uint32_t l = (uint32_t)(normalised_dividend);
    asm("ldivu %0,%1,%2,%3,%4":"=r"(quotient):"r"(remainder),"r"(h),"r"(l),"r"(h_divisor));
#else
    uint64_t quotient = (uint64_t)(normalised_dividend) / h_divisor;
#endif

    res.exp = dividend_exp - divisor_exp - lhs;

    res.mant = (uint32_t)(quotient) ;
    return res;
}

typedef struct
{
    uint64_t mant;
    int32_t exp;
}float_u64_t;

static float_u64_t float_div_u64(float_s32_t dividend, float_s32_t divisor)
{
    float_u64_t res;

    int dividend_hr;
    int divisor_hr;

#if __xcore__
    asm( "clz %0, %1" : "=r"(dividend_hr) : "r"(dividend.mant) );
    asm( "clz %0, %1" : "=r"(divisor_hr) : "r"(divisor.mant) );
#else
    dividend_hr = __builtin_clz(dividend.mant);
    divisor_hr =  __builtin_clz(divisor.mant);
#endif

    int dividend_exp = dividend.exp - dividend_hr;
    int divisor_exp = divisor.exp - divisor_hr;

    uint64_t h_dividend = (uint64_t)((uint32_t)dividend.mant) << (dividend_hr);

    uint32_t h_divisor = ((uint32_t)divisor.mant) << (divisor_hr);

    uint32_t lhs = 32;

    uint64_t quotient = (h_dividend << lhs) / h_divisor;

    res.exp = dividend_exp - divisor_exp - lhs;

    res.mant = quotient ;
    return res;
}

uint64_t float_div_u64_fixed_output_q_format(float_s32_t dividend, float_s32_t divisor, int32_t output_q_format)
{
    int op_q = -output_q_format;
    float_u64_t res = float_div_u64(dividend, divisor);
    uint64_t quotient;
    if(res.exp < op_q)
    {
        int rsh = op_q - res.exp;
        quotient = ((uint64_t)res.mant >> rsh) + (((uint64_t)res.mant >> (rsh-1)) & 0x1);
    }
    else
    {
        int lsh = res.exp - op_q;
        quotient = (uint64_t)res.mant << lsh;
    }
    return quotient;
}
