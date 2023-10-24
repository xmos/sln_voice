// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include "i2s.h"

I2S::I2S(sc_module_name name, Buffer* buffer, config_t *config)
    : sc_module(name)
    , m_buffer(buffer)
    , m_config(config)
    , clk("clk")
{
    SC_THREAD(process); sensitive << clk.pos();
}

void I2S::process()
{
    while(true)
    {
        wait();
        m_buffer->read(1);
    }
}
