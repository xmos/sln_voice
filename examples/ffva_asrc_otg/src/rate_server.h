#ifndef RATE_SERVER_H
#define RATE_SERVER_H

void rate_server(void *args);

#define SAMPLING_RATE_Q_FORMAT (23)

typedef struct
{
    /* data */
    uint32_t usb_data_rate;
    int32_t samples_to_host_buf_fill_level;
    bool mic_itf_open;
    bool spkr_itf_open;
}usb_to_i2s_rate_info_t;

// Extern variables
extern uint32_t g_i2s_to_usb_rate_ratio;
extern uint32_t g_i2s_nominal_sampling_rate;

#endif
