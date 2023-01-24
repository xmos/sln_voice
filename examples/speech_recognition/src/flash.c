// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <platform.h>
#include <stdio.h>
#include <xmos_flash.h>

flash_ports_t flash_ports_0 =
{
  PORT_SQI_CS,
  PORT_SQI_SCLK,
  PORT_SQI_SIO,
  XS1_CLKBLK_5
};

flash_clock_config_t flash_clock_config =
{
  1,
  8,
  8,
  1,
  0,
};

flash_qe_config_t flash_qe_config_0 =
{
  flash_qe_location_status_reg_0,
  flash_qe_bit_6
};

flash_handle_t flash_handle;

void flash_setup() {
    flash_connect(&flash_handle, &flash_ports_0, flash_clock_config, flash_qe_config_0);
}

void flash_teardown() {
    flash_disconnect(&flash_handle);
}

void flash_read_wrapper(unsigned src, unsigned *dest, size_t num_words) {
    flash_read_quad(&flash_handle, src, dest, num_words);
}