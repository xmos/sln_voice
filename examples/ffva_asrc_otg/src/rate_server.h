#ifndef RATE_SERVER_H
#define RATE_SERVER_H
#include "xmath/xmath.h"

void rate_server(void *args);
float_s32_t my_ema_calc(float_s32_t x, float_s32_t y, uint32_t alpha_q30, int32_t output_exp);
uint32_t my_ema_calc_custom(uint32_t x, uint32_t y, int input_exp, uint32_t alpha_q31, int32_t output_exp);

#define SAMPLING_RATE_Q_FORMAT (23)

typedef struct
{
    /* data */
    float_s32_t usb_data_rate;
    int32_t samples_to_host_buf_fill_level;
    bool mic_itf_open;
    bool spkr_itf_open;
}usb_to_i2s_rate_info_t;

typedef struct
{
    /* data */
    uint32_t usb_to_i2s_rate_ratio;
}i2s_to_usb_rate_info_t;


// Extern variables
extern uint32_t g_i2s_to_usb_rate_ratio;
extern uint32_t g_i2s_nominal_sampling_rate;

#endif
