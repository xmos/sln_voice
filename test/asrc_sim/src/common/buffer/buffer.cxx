// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include "buffer.h"

Buffer::Buffer(sc_module_name name)
    : sc_module(name)
{
}

void Buffer::write(unsigned int num_samples)
{
    m_buffer_level += num_samples;
    m_samples_written += num_samples;
    assert(m_buffer_level < MAX_LEVEL);
}

void Buffer::read(unsigned int num_samples)
{
    m_buffer_level -= num_samples;
    //printf("m_buffer_level = %d\n", m_buffer_level);
    m_samples_read += num_samples;
    assert(m_buffer_level > -MAX_LEVEL);
}

int Buffer::fill_level()
{
    return m_buffer_level;
}

void sc_trace(sc_trace_file* f, const Buffer& val, const std::string &name)
{
    sc_trace(f, val.m_buffer_level, name + ".buffer_level");
}
