// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#pragma once

#include "systemc.h"

SC_MODULE(Buffer)
{
    public:
        SC_CTOR(Buffer);
        friend void sc_trace(sc_trace_file* f, const Buffer& val, const std::string &name);

    private:
        int m_buffer_level = 0;
        const int MAX_LEVEL = 240*4;
        int m_samples_written = 0;
        int m_samples_read = 0;

    public:
        void write(unsigned int num_samples);
        void read(unsigned int num_samples);
        int fill_level(void);
};
