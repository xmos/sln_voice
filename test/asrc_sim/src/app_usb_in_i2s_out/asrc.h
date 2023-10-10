// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#pragma once

#include "systemc.h"
#include "buffer.h"
#include "ASRC_wrapper.h"
#include "config.h"

SC_MODULE(ASRC)
{
    public:
        ASRC(sc_module_name name, uint32_t fs_in, uint32_t fs_out, uint32_t block_size, double actual_rate_ratio, Buffer* buffer, sc_event& trigger, config_t *config);
        SC_HAS_PROCESS(ASRC);

    public:
        sc_event& m_trigger;

    private:
        Buffer* m_buffer = nullptr;
        config_t *m_config = nullptr;
        double m_nominal_rate_ratio_f;
        uint64_t m_nominal_rate_ratio;

        double m_actual_rate_ratio_f;
        uint64_t m_actual_rate_ratio;

        uint32_t m_block_size;

        ASRCCtrl_profile_only_t *m_profile_info_ptr[MAX_ASRC_N_IO_CHANNELS];

    public:
        void process();
};
