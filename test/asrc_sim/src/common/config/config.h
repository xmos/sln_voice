// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#pragma once

typedef struct
{
    /* data */
    double nominal_i2s_rate;
    double nominal_usb_rate;
    double actual_usb_rate;
    double average_usb_rate_from_sofs;
    double usb_drift_ppm;
    double asrc_input_sine_freq;
    int asrc_block_size;
    std::vector<uint32_t> usb_timestamps[2]; // 2 in case OUT and IN timestamps are present.
    int *asrc_input_samples;
}config_t;
