// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#pragma once

#include <xcore/parallel.h>
#include <xcore/channel.h>

#include "i2c.h"


DECLARE_JOB(i2c_control, (chanend_t));
void i2c_control(chanend_t c_i2c_reg);