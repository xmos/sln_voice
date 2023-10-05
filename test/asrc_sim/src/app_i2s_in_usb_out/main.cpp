// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <string>
#include <sstream>
#include <fstream>
#include "systemc.h"
#include "buffer.h"
#include "usb.h"
#include "i2s.h"
#include "asrc.h"
#include "config.h"
#include "helpers.h"

#define DEFAULT_NOMINAL_USB_RATE (48000) // Do not change!! Only 48000KHz USB supported
#define DEFAULT_USB_DRIFT_PPM    (10)
#define DEFAULT_SIM_TIME_MINS    (20)    // Simulation time in mins
#define ASRC_BLOCK_SIZE          (244)   // Number of samples that make the ASRC input block

// Usage. From the build directory, run: ./i2s_in_usb_out <i2s_rate> ../log_sofs_1hr 2>&1 | tee log
int sc_main(int argc, char* argv[])
{
    config_t *app_config = new config_t;
    app_config->nominal_usb_rate = DEFAULT_NOMINAL_USB_RATE;
    app_config->usb_drift_ppm = DEFAULT_USB_DRIFT_PPM;
    app_config->asrc_block_size = ASRC_BLOCK_SIZE;
    // Choose the frequency of the sine tone used as ASRC input such that there are an integer no. of periods in a 128 point FFT on the asrc output, which is at the USB rate
    app_config->asrc_input_sine_freq = 6000;

    if(argc < 2)
    {
        printf("Usage:\ni2s_in_usb_out <i2s_rate> \nor\ni2s_in_usb_out <i2s_rate> <USB timestamps file>\nExiting\n");
        return -1;
    }
    app_config->nominal_i2s_rate = (double)(atoi(argv[1]));
    if(verify_i2s_rate(app_config->nominal_i2s_rate) != 0)
    {
        return -1;
    }

    if(argc == 3) // If SOF timestamps file is provided, parse the timestamps into a std::vector
    {
        printf("argv[2] = %s\n", argv[2]);
        parse_sof_timestamps(argv[2], app_config);
    }

    app_config->actual_usb_rate = (double)app_config->nominal_usb_rate * (1 + app_config->usb_drift_ppm/1000000);
    double sof_period = (1e-3/(1/app_config->nominal_i2s_rate)) / (1 + app_config->usb_drift_ppm/1000000);

    double i2s_asrc_input_block_period = ((app_config->asrc_block_size/app_config->nominal_i2s_rate))/(1/app_config->nominal_i2s_rate);
    printf("SOF period = %f\n", sof_period);

    double actual_rate_ratio = app_config->nominal_i2s_rate / app_config->actual_usb_rate;

    sc_clock usb_clk("usb_clk", sof_period, SC_US);
    sc_clock i2s_clk("i2s_clk", i2s_asrc_input_block_period, SC_US);

    app_config->asrc_input_samples = new int[app_config->asrc_block_size * 2];

    Buffer buffer("buffer");
    USB usb("usb", &buffer, app_config);
    I2S i2s("i2s", &buffer, app_config);
    ASRC asrc("asrc", (uint32_t)app_config->nominal_i2s_rate, (uint32_t)app_config->nominal_usb_rate, app_config->asrc_block_size, actual_rate_ratio, &buffer, i2s.trigger, app_config);


    usb.clk(usb_clk);
    i2s.clk(i2s_clk);

    // Initialise the simulation
    sc_start(0, SC_SEC);


    // Simulate for N seconds.
    sc_start(DEFAULT_SIM_TIME_MINS*60*app_config->nominal_i2s_rate, SC_US);

    delete app_config->asrc_input_samples;
    delete app_config;

    return 0;
}
