// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#pragma once

#include "systemc.h"
#include "buffer.h"
#include "config.h"

SC_MODULE(USB)
{
    public:
        USB(sc_module_name name, Buffer* buffer, config_t *config);
        SC_HAS_PROCESS(USB);

    public:
        sc_in<bool> clk;
        sc_event trigger;

    private:
        Buffer* m_buffer = nullptr;
        config_t *m_config;

    public:
        void process();
};
