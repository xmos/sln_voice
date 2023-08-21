// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#pragma once


DECLARE_JOB(tdm16_master_simple, (void));
void tdm16_master_simple(void);

// Temp monitor task
DECLARE_JOB(tdm_master_monitor, (void));
void tdm_master_monitor(void);
