// Copyright 2023-2026 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <platform.h>
#include <xs1.h>
#include <stdio.h>
#include <xscope.h>
//#include <xcore/channel.h>

//void xscope_user_init(void) {
//    xscope_register(0, 0, "", 0, "");
//    xscope_config_io(XSCOPE_IO_BASIC);
//}

extern "C" {
    void main_tile_0(chanend c_cross_tile[2]);
    void main_tile_1(chanend c_cross_tile[2]);
}

on tile[0]: in port p_vbus = XS1_PORT_8D;

static unsigned vbus_state_tile1 = 0;
static unsigned previous_vbus_state_tile1 = 0;

unsigned vbus_state = 0;

void vbus_sampler_task(chanend c) {
  unsigned v;
  
  while (1) {
    p_vbus :> v;
    vbus_state = (v >> 6) & 1;
    unsigned char bit = (unsigned char)(vbus_state);
    c <: bit;
  }
}

void vbus_rx_task(chanend c) {
    unsigned char bit;

    while (1) {
        select {
            case c :> bit:
                vbus_state_tile1 = (unsigned) bit;
                break;
        }
    }
}

int main(void)
{
    chan c_cross_tile[2];
    // Cross-tile VBUS channel
    chan c_vbus;


    /* 'Par' statement to run the following tasks in parallel */
    par
    {
        on tile[0]: {
            par {
                vbus_sampler_task(c_vbus);
                main_tile_0(c_cross_tile);
            }

        }
        on tile[1]: {
            par {
                vbus_rx_task(c_vbus);
                main_tile_1(c_cross_tile);
            }
        }
    }
    return 0;
}

unsigned int XUD_HAL_GetVBusState(void)
{
    if(previous_vbus_state_tile1 != vbus_state_tile1) {
        if(vbus_state_tile1 == 0) {
            printf("USB disconnected\n");
        } else {
            printf("USB connected\n");
        }
        previous_vbus_state_tile1 = vbus_state_tile1;
    }
    return vbus_state_tile1;
}
