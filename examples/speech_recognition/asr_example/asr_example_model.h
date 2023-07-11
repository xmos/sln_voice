// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef SIMPLE_ASR_MODEL_H
#define SIMPLE_ASR_MODEL_H

#include <platform.h>

#include "qspi_flash_fast_read.h"

// TODO: Set your model (and optional grammar) data here

// Use this to define where your model is placed in flash
//   XS1_SWMEM_BASE is the base address for flash
//   QFFR_DEFAULT_CAL_PATTERN_BUF_SIZE_WORDS is the size of the flash calibration data (in words)
#define MODEL_FLASH_OFFSET  (XS1_SWMEM_BASE + (QFFR_DEFAULT_CAL_PATTERN_BUF_SIZE_WORDS*sizeof(int)))

int32_t *model_data = (int32_t *) (MODEL_FLASH_OFFSET);

// OR, use the line below to include a model placed in SRAM
//#include "asr_example_model.c"

// This simple ASR port is not using grammar data
int32_t *grammar_data = NULL; 

#endif // SIMPLE_ASR_MODEL_H