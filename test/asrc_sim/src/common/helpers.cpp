
#include <string>
#include <sstream>
#include <fstream>
#include "systemc.h"
#include "config.h"

#include <cstdlib>
#include <iostream>

#define PARSE_SOF_TIMESTAMPS (1)

void parse_sof_timestamps(const char *fname, config_t *app_config)
{
    ifstream myfile;
    std::ifstream infile(fname);
    std::string line;

    std::string token;
    while (std::getline(infile, line))
    {
#if PARSE_SOF_TIMESTAMPS
        app_config->usb_timestamps[0].push_back(std::stoul(line.c_str()));
        app_config->usb_timestamps[0].push_back(int(384));  // Data transferred is always 384 bytes (48, 32bit, 2ch samples per 1ms).
#else
        std::istringstream ss(line);
        int count = 0;
        int dir;
        while(std::getline(ss, token, ','))
        {
            if(count == 0)
            {
                dir = std::stoul(token); // Set direction based on first token
            }
            else
            {
                app_config->usb_timestamps[dir].push_back(std::stoul(token));
            }
            count += 1;
        }
#endif
    }
    printf("usb_timestamps[0].size() = %lu\n", app_config->usb_timestamps[0].size());
    printf("usb_timestamps[1].size() = %lu\n", app_config->usb_timestamps[1].size());
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
