// Copyright 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <platform.h>
#include <stdint.h>
#include <stdio.h>

#include <xcore/assert.h>

#include "qspi_flash_fast_read.h"

#define CLK_DIVIDE          3  // for XK_VOICE_L71
//#define CLK_DIVIDE          4  // for XCORE_AI_EXPLORER

qspi_fast_flash_read_ctx_t qspi_fast_flash_read_ctx;
qspi_fast_flash_read_ctx_t *ctx = &qspi_fast_flash_read_ctx;

void flash_setup() {
    qspi_flash_fast_read_init(ctx,
        XS1_CLKBLK_1,
        XS1_PORT_1B,
        XS1_PORT_1C,
        XS1_PORT_4B,
        qspi_fast_flash_read_transfer_nibble_swap,
        CLK_DIVIDE
    );
    
    qspi_flash_fast_read_setup_resources(ctx);

    uint32_t addr = 0x00000000;
    uint32_t scratch_buf[QFFR_DEFAULT_CAL_PATTERN_BUF_SIZE_WORDS];

    if (qspi_flash_fast_read_calibrate(ctx, addr, qspi_flash_fast_read_pattern_expect_default, scratch_buf, QFFR_DEFAULT_CAL_PATTERN_BUF_SIZE_WORDS) != 0) {
        printf("Fast flash calibration failed\n"); 
    }
}

void flash_teardown() {
  xassert(ctx);
  qspi_flash_fast_read_shutdown(ctx);
}

void flash_read_wrapper(unsigned *dest, unsigned src, size_t size) {
  xassert(ctx);
  qspi_flash_fast_read(ctx, src, (uint8_t *)dest, size);
}