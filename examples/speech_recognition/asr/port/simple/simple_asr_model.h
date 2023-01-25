// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef SIMPLE_ASR_MODEL_H
#define SIMPLE_ASR_MODEL_H

// TODO: Set your model (and optional grammar) data here

// Use this to define where your model is placed in flash
int32_t *model_data = (int32_t *) (XS1_SWMEM_BASE + 0x100000); // XS1_SWMEM_BASE is the base address for flash
                                                               // 0x100000 is the flashed boot partition size

// OR, use the line below to include a model placed in SRAM
//#include "simple_asr_model.c"

// This simple ASR port is not using grammar data
int32_t *grammar_data = NULL; 

#endif // SIMPLE_ASR_MODEL_H