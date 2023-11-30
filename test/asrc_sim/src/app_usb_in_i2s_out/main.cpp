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
#define ASRC_BLOCK_SIZE          (96)    // Number of samples that make the ASRC input block


// Usage. From the build directory, run: ./usb_in_i2s_out <i2s_rate> ../log_sofs_1hr 2>&1 | tee log
int sc_main(int argc, char* argv[])
{
    config_t *app_config = new config_t;
    app_config->nominal_usb_rate = DEFAULT_NOMINAL_USB_RATE;
    app_config->usb_drift_ppm = DEFAULT_USB_DRIFT_PPM;
    app_config->asrc_block_size = ASRC_BLOCK_SIZE;


    if(argc < 2)
    {
        printf("Usage:\nusb_in_i2s_out <i2s_rate> \nor\nusb_in_i2s_out <i2s_rate> <USB timestamps file>\nExiting\n");
        return -1;
    }
    app_config->nominal_i2s_rate = (double)(atoi(argv[1]));
    if(verify_i2s_rate(app_config->nominal_i2s_rate) != 0)
    {
        return -1;
    }

    // Choose the frequency of the sine tone used as ASRC input such that there are an integer no. of periods in a 128 point FFT on the asrc output, which is at the I2s rate
    if((app_config->nominal_i2s_rate == 192000) || (app_config->nominal_i2s_rate == 96000) || (app_config->nominal_i2s_rate == 48000)) // for the 48 family
    {
        app_config->asrc_input_sine_freq = 12000;
    }
    else // for 44.1 family
    {
        app_config->asrc_input_sine_freq = 11025;
    }

    if(app_config->asrc_block_size % 48 != 0)
    {
        printf("ERROR: asrc_block_size of %d is not a multiple of the USB frame size of 48 samples\n", app_config->asrc_block_size);
        return -1;
    }

    if(argc == 3) // If SOF timestamps file is provided, parse the timestamps into a std::vector
    {
        printf("argv[2] = %s\n", argv[2]);
        parse_sof_timestamps(argv[2], app_config);
    }

    app_config->actual_usb_rate = (double)app_config->nominal_usb_rate * (1 + app_config->usb_drift_ppm/1000000);
    double sof_period = (1e-3/(1/app_config->nominal_i2s_rate)) / (1 + app_config->usb_drift_ppm/1000000);
    printf("SOF period = %f\n", sof_period);

    double actual_rate_ratio =  app_config->actual_usb_rate / app_config->nominal_i2s_rate;

    sc_clock usb_clk("usb_clk", sof_period, SC_US);
    sc_clock i2s_clk("i2s_clk", 1, SC_US);

    app_config->asrc_input_samples = new int[app_config->asrc_block_size];

    Buffer buffer("buffer");
    USB usb("usb", &buffer, app_config);
    ASRC asrc("asrc", (uint32_t)app_config->nominal_usb_rate, (uint32_t)app_config->nominal_i2s_rate, app_config->asrc_block_size, actual_rate_ratio, &buffer, usb.trigger, app_config);
    I2S i2s("i2s", &buffer, app_config);

    usb.clk(usb_clk);
    i2s.clk(i2s_clk);

    // Initialise the simulation
    sc_start(0, SC_SEC);

    // Simulate for N seconds
    sc_start(DEFAULT_SIM_TIME_MINS*60*app_config->nominal_i2s_rate, SC_US);

    return 0;
}
