// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#pragma once

#include "app_main.h"           // audio_frame_t
#include "i2s_tdm_slave.h"


DECLARE_JOB(tdm16_slave, (audio_frame_t **));
void tdm16_slave(audio_frame_t **read_buffer);