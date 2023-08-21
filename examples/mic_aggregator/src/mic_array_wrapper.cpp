// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <stdint.h>
#include <xcore/channel_streaming.h>
#include <xcore/interrupt.h>

#include "app_config.h"
#include "app_mic_array.hpp"

#include "mic_array.h"
#include "mic_array/etc/filters_default.h"
#include "mic_array_48k_decimator_coeffs.h"

#ifndef STR
#define STR(s) #s
#endif

#ifndef XSTR
#define XSTR(s) STR(s)
#endif

#ifndef MIC_ARRAY_CONFIG_USE_DC_ELIMINATION
# define MIC_ARRAY_CONFIG_USE_DC_ELIMINATION    (1)
#endif

#ifndef MIC_ARRAY_CONFIG_CLOCK_BLOCK_A
# define MIC_ARRAY_CONFIG_CLOCK_BLOCK_A         (XS1_CLKBLK_1)
#endif

#ifndef MIC_ARRAY_CONFIG_CLOCK_BLOCK_B
# define MIC_ARRAY_CONFIG_CLOCK_BLOCK_B         (XS1_CLKBLK_2)
#endif

////// Additional macros derived from others

#define MIC_ARRAY_CONFIG_MCLK_DIVIDER           ((MIC_ARRAY_CONFIG_MCLK_FREQ)       \
                                                /(MIC_ARRAY_CONFIG_PDM_FREQ))

#define MIC_ARRAY_CONFIG_OUT_SAMPLE_RATE        ((MIC_ARRAY_CONFIG_PDM_FREQ)      \
                                                /(STAGE2_DEC_FACTOR))

////// Any Additional correctness checks


////// Allocate needed objects

pdm_rx_resources_t pdm_res = PDM_RX_RESOURCES_DDR(
                                MIC_ARRAY_CONFIG_PORT_MCLK,
                                MIC_ARRAY_CONFIG_PORT_PDM_CLK,
                                MIC_ARRAY_CONFIG_PORT_PDM_DATA,
                                MIC_ARRAY_CONFIG_CLOCK_BLOCK_A,
                                MIC_ARRAY_CONFIG_CLOCK_BLOCK_B);

constexpr int mic_count = MIC_ARRAY_CONFIG_MIC_COUNT;

static const uint32_t WORD_ALIGNED stage1_coef_custom[128] = STAGE_1_48K_COEFFS;
static const int32_t WORD_ALIGNED stage2_coef_custom[MIC_ARRAY_STAGE_2_NUM_TAPS] = STAGE_2_48K_COEFFS;
static constexpr right_shift_t stage2_shift_custom = MIC_ARRAY_CONFIG_STG2_RIGHT_SHIFT;

constexpr const uint32_t* stage_1_filter() {
    return &stage1_coef_custom[0];
}

constexpr int decimation_factor = MIC_ARRAY_CONFIG_STG2_DEC_FACTOR;
constexpr int stage_2_tap_count = MIC_ARRAY_STAGE_2_NUM_TAPS;

constexpr const int32_t* stage_2_filter() {
    return &stage2_coef_custom[0];
}
constexpr const right_shift_t* stage_2_shift() {
    return &stage2_shift_custom;
}

using TMicArray = mic_array::MicArray<mic_count,
                          par_mic_array::MyTwoStageDecimator<mic_count, 
                                                       decimation_factor, 
                                                       stage_2_tap_count>,
                          mic_array::StandardPdmRxService<mic_count,
                                                          mic_count,
                                                          decimation_factor>, 
                          // std::conditional uses USE_DCOE to determine which 
                          // sample filter is used.
                          typename std::conditional<MIC_ARRAY_CONFIG_USE_DC_ELIMINATION,
                                              mic_array::DcoeSampleFilter<mic_count>,
                                              mic_array::NopSampleFilter<mic_count>>::type,
                          mic_array::FrameOutputHandler<mic_count, 
                                                        MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME, 
                                                        mic_array::ChannelFrameTransmitter>>;

TMicArray mics;

MA_C_API
void app_mic_array_init()
{
  mic_array_resources_configure(&pdm_res, MIC_ARRAY_CONFIG_MCLK_DIVIDER);

  mics.Decimator.Init(stage_1_filter(), stage_2_filter(), *stage_2_shift());
  mics.PdmRx.Init(pdm_res.p_pdm_mics);

  printf("MIC CONFIG:\n");
  printf("- MIC_ARRAY_TILE: " XSTR(MIC_ARRAY_TILE) "\n");
  printf("- MIC_ARRAY_CONFIG_CLOCK_BLOCK_A: " XSTR(MIC_ARRAY_CONFIG_CLOCK_BLOCK_A) "\n");
  printf("- MIC_ARRAY_CONFIG_CLOCK_BLOCK_B: " XSTR(MIC_ARRAY_CONFIG_CLOCK_BLOCK_B) "\n");
  printf("- MIC_ARRAY_CONFIG_MCLK_FREQ: " XSTR(MIC_ARRAY_CONFIG_MCLK_FREQ) "\n");
  printf("- MIC_ARRAY_CONFIG_PDM_FREQ: " XSTR(MIC_ARRAY_CONFIG_PDM_FREQ) "\n");
  printf("- MIC_ARRAY_CONFIG_MIC_COUNT: " XSTR(MIC_ARRAY_CONFIG_MIC_COUNT) "\n");
  printf("- MIC_ARRAY_CONFIG_USE_DDR: " XSTR(MIC_ARRAY_CONFIG_USE_DDR) "\n");
  printf("- MIC_ARRAY_CONFIG_PORT_MCLK: " XSTR(MIC_ARRAY_CONFIG_PORT_MCLK) "\n");
  printf("- MIC_ARRAY_CONFIG_PORT_PDM_CLK: " XSTR(MIC_ARRAY_CONFIG_PORT_PDM_CLK) "\n");
  printf("- MIC_ARRAY_CONFIG_MCLK_DIVIDER: " XSTR(MIC_ARRAY_CONFIG_MCLK_DIVIDER) "\n");
  printf("- MIC_ARRAY_CONFIG_PORT_PDM_DATA: " XSTR(MIC_ARRAY_CONFIG_PORT_PDM_DATA) "\n");
  printf("- MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME: " XSTR(MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME) "\n");
  printf("- MIC_ARRAY_NUM_DECIMATOR_TASKS: " XSTR(MIC_ARRAY_NUM_DECIMATOR_TASKS) "\n");
  printf("- MIC_ARRAY_PDM_RX_OWN_THREAD: " XSTR(MIC_ARRAY_PDM_RX_OWN_THREAD) "\n");
  
  if(!MIC_ARRAY_PDM_RX_OWN_THREAD)
  {
    mic_array_pdm_clock_start(&pdm_res);
  }
}

MA_C_API
void app_pdm_rx_task()
{
  mic_array_pdm_clock_start(&pdm_res);
  mics.PdmRx.ThreadEntry();
}

MA_C_API
void app_mic_array_task(chanend_t c_frames_out)
{
  mics.OutputHandler.FrameTx.SetChannel(c_frames_out);

  if(!MIC_ARRAY_PDM_RX_OWN_THREAD)
  {
    mics.PdmRx.InstallISR();
    mics.PdmRx.UnmaskISR();
  }
  mics.ThreadEntry();
}

MA_C_API
void app_mic_array_assertion_disable()
{
  mics.PdmRx.AssertOnDroppedBlock(false);
}

MA_C_API
void app_mic_array_assertion_enable()
{
  mics.PdmRx.AssertOnDroppedBlock(true);
}
