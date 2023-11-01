// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <string>
#include <sstream>
#include <fstream>
#include "systemc.h"
#include "config.h"

#include <cstdlib>
#include <iostream>
#include <numeric>



void parse_sof_timestamps(const char *fname, config_t *app_config)
{
    ifstream myfile;
    std::ifstream infile(fname);
    std::string line;

    std::string token;
    std::vector<uint32_t> only_timestamps;
    while (std::getline(infile, line))
    {

        app_config->usb_timestamps[0].push_back(std::stoul(line.c_str()));
        app_config->usb_timestamps[0].push_back(int(384));  // Data transferred is always 384 bytes (48, 32bit, 2ch samples per 1ms).

        only_timestamps.push_back(std::stoul(line.c_str()));
    }
    printf("usb_timestamps[0].size() = %lu\n", app_config->usb_timestamps[0].size());
    if (only_timestamps.size())
    {
        uint64_t accum = 0;
        uint32_t counter = 0;
        for (int i=0; i<only_timestamps.size()-1; i++)
        {
           uint32_t span =  (uint32_t)only_timestamps[i+1] - (uint32_t)only_timestamps[i];
           counter += 1;
           accum = accum + span;
        }
        double avg = (double)accum / counter;
        double avg_usb_rate = (48 / avg) * 100000000;   // USB rate is samples per second. (samples_per_tick * ticks_per_second)
        app_config->average_usb_rate_from_sofs = avg_usb_rate;
    }
    else
    {
        app_config->average_usb_rate_from_sofs = 48000;
    }

}

int verify_i2s_rate(int i2s_rate)
{
    if((i2s_rate != 192000) && (i2s_rate != 176400)
       && (i2s_rate != 96000) && (i2s_rate != 88200)
       && (i2s_rate != 48000) && (i2s_rate != 44100))
    {
        printf("ERROR: Invalid sampling rate %d\n", (int)i2s_rate);
        return -1;
    }
    printf("nominal_i2s_rate = %d\n", (int)i2s_rate);
    return 0;
}
