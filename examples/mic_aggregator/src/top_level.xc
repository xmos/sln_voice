// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <platform.h>
#include <xs1.h>

extern void main_tile_0(chanend c_cross_tile[2]);
extern void main_tile_1(chanend c_cross_tile[2]);

int main() {
    chan c_cross_tile[2];

    /* 'Par' statement to run the following tasks in parallel */
    par
    {
        on tile[0]: main_tile_0(c_cross_tile);
        on tile[1]: main_tile_1(c_cross_tile);
    }
    return 0;
}
