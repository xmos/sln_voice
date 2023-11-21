// Copyright 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "platform/driver_instances.h"

static rtos_intertile_t intertile_ctx_s;
rtos_intertile_t *intertile_ctx = &intertile_ctx_s;

static rtos_intertile_t intertile_usb_audio_ctx_s;
rtos_intertile_t *intertile_usb_audio_ctx = &intertile_usb_audio_ctx_s;

static rtos_intertile_t intertile_i2s_audio_ctx_s;
rtos_intertile_t *intertile_i2s_audio_ctx = &intertile_i2s_audio_ctx_s;

static rtos_qspi_flash_t qspi_flash_ctx_s;
rtos_qspi_flash_t *qspi_flash_ctx = &qspi_flash_ctx_s;

static rtos_i2c_master_t i2c_master_ctx_s;
rtos_i2c_master_t *i2c_master_ctx = &i2c_master_ctx_s;

static rtos_i2s_t i2s_ctx_s;
rtos_i2s_t *i2s_ctx = &i2s_ctx_s;

static rtos_dfu_image_t dfu_image_ctx_s;
rtos_dfu_image_t *dfu_image_ctx = &dfu_image_ctx_s;
