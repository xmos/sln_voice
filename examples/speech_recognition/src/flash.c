// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <platform.h>
#include <stdint.h>
#include <stdio.h>

#include <xmos_flash.h>

#define BYTES_TO_WORDS(b) (((b) + sizeof(uint32_t) - 1) / sizeof(uint32_t))
#define BYTE_TO_WORD_ADDRESS(b) ((b) / sizeof(uint32_t))

flash_ports_t flash_ports_0 =
{
  PORT_SQI_CS,
  PORT_SQI_SCLK,
  PORT_SQI_SIO,
  XS1_CLKBLK_5
};

// use the flash clock config below to get 50MHz, ~23.8 MiB/s throughput
static flash_clock_config_t flash_clock_config = {
    flash_clock_reference,  0, 1, flash_clock_input_edge_plusone,
    flash_port_pad_delay_1,
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

void flash_read_wrapper(unsigned *dest, unsigned src, size_t size) {
  // NOTE: flash_read_quad
  //    has src and dest parameters in different order
  //    expects src address to be offset from the XS1_SWMEM_BASE address
  //    expects src address to be word address
  //    expects size to be in words, not bytes
  flash_read_quad(&flash_handle, BYTE_TO_WORD_ADDRESS(src - XS1_SWMEM_BASE), dest, BYTES_TO_WORDS(size));
}