// Copyright 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <platform.h>
#include <xs1.h>
#include <stdio.h>
#include <xscope.h>
#include <stdlib.h>

#include "xscope_io_device.h"

extern "C" {
    void tile0_main();
}

int main() {
    chan xscope_chan;

    par {
        xscope_host_data(xscope_chan);
        on tile[0]: 
        {
          xscope_io_init(xscope_chan);
          tile0_main();
        }
        on tile[1]:
        {
            // Nothing running on tile[1]
        }
    }
    return 0;
}