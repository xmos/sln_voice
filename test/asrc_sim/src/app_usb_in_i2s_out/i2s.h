// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#pragma once

#include "systemc.h"
#include "buffer.h"
#include "config.h"

SC_MODULE(I2S)
{
    public:
        I2S(sc_module_name name, Buffer* buffer, config_t *config);
        SC_HAS_PROCESS(I2S);

    public:
        sc_in<bool> clk;

    private:
        Buffer* m_buffer = nullptr;
        config_t *m_config = nullptr;

    public:
        void process();
};
