// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <stdint.h>
#include <xcore/channel_streaming.h>
#include <xcore/interrupt.h>

#include "mic_array/cpp/Prefab.hpp"
#include "mic_array.h"
#include "mic_array/etc/filters_default.h"
#include "mic_array_custom.h"

////// Check that all the required config macros have been defined.

#ifndef MIC_ARRAY_CONFIG_MCLK_FREQ
# error Application must specify the master clock frequency by defining MIC_ARRAY_CONFIG_MCLK_FREQ.
#endif

#ifndef MIC_ARRAY_CONFIG_PDM_FREQ
# error Application must specify the PDM clock frequency by defining MIC_ARRAY_CONFIG_PDM_FREQ.
#endif

#ifndef MIC_ARRAY_CONFIG_MIC_COUNT
# error Application must specify the microphone count by defining MIC_ARRAY_CONFIG_MIC_COUNT.
#endif


////// Provide default values for optional config macros

#ifndef MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME
# define MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME    (1)
#else
# if ((MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME) < 1)
#  error MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME must be positive.
# endif
#endif

#ifndef MIC_ARRAY_CONFIG_USE_DC_ELIMINATION
# define MIC_ARRAY_CONFIG_USE_DC_ELIMINATION    (1)
#endif

#ifndef MIC_ARRAY_CONFIG_PORT_MCLK
# define MIC_ARRAY_CONFIG_PORT_MCLK   (PORT_MCLK_IN_OUT)
#endif

#ifndef MIC_ARRAY_CONFIG_PORT_PDM_CLK
# define MIC_ARRAY_CONFIG_PORT_PDM_CLK  (PORT_PDM_CLK)
#endif

#ifndef MIC_ARRAY_CONFIG_PORT_PDM_DATA
# define MIC_ARRAY_CONFIG_PORT_PDM_DATA   (PORT_PDM_DATA)
#endif

#ifndef MIC_ARRAY_CONFIG_CLOCK_BLOCK_A
# define MIC_ARRAY_CONFIG_CLOCK_BLOCK_A   (XS1_CLKBLK_1)
#endif

#ifndef MIC_ARRAY_CONFIG_CLOCK_BLOCK_B
# define MIC_ARRAY_CONFIG_CLOCK_BLOCK_B   (XS1_CLKBLK_2)
#endif

#ifndef MIC_ARRAY_CONFIG_USE_DDR
# define MIC_ARRAY_CONFIG_USE_DDR         ((MIC_ARRAY_CONFIG_MIC_COUNT)==2)
#endif

////// Additional macros derived from others

#define MIC_ARRAY_CONFIG_MCLK_DIVIDER     ((MIC_ARRAY_CONFIG_MCLK_FREQ)       \
                                              /(MIC_ARRAY_CONFIG_PDM_FREQ))
#define MIC_ARRAY_CONFIG_OUT_SAMPLE_RATE    ((MIC_ARRAY_CONFIG_PDM_FREQ)      \
                                              /(STAGE2_DEC_FACTOR))

////// Any Additional correctness checks



////// Allocate needed objects

#if (!(MIC_ARRAY_CONFIG_USE_DDR))
pdm_rx_resources_t pdm_res_custom = PDM_RX_RESOURCES_SDR(
                                MIC_ARRAY_CONFIG_PORT_MCLK,
                                MIC_ARRAY_CONFIG_PORT_PDM_CLK,
                                MIC_ARRAY_CONFIG_PORT_PDM_DATA,
                                MIC_ARRAY_CONFIG_CLOCK_BLOCK_A);
#else
pdm_rx_resources_t pdm_res_custom = PDM_RX_RESOURCES_DDR(
                                MIC_ARRAY_CONFIG_PORT_MCLK,
                                MIC_ARRAY_CONFIG_PORT_PDM_CLK,
                                MIC_ARRAY_CONFIG_PORT_PDM_DATA,
                                MIC_ARRAY_CONFIG_CLOCK_BLOCK_A,
                                MIC_ARRAY_CONFIG_CLOCK_BLOCK_B);
#endif


// Designed from https://github.com/xmos/lib_mic_array/tree/develop/script/filter_design
// python3 design_filter.py
// python3 ../stage1.py good_48k_filter_int.pkl
// python3 ../stage2.py good_48k_filter_int.pkl

#define MIC_ARRAY_48K_STAGE_1_TAP_COUNT 148
#define MIC_ARRAY_48K_STAGE_1_FILTER_WORD_COUNT 128
static const uint32_t WORD_ALIGNED stage1_48k_coefs[MIC_ARRAY_48K_STAGE_1_FILTER_WORD_COUNT]
{
  0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFF2DBBA, 0x1E443FC2, 0x2788F9F1, 0x1E443FC2, 0x2785DDB4,
  0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFF86BEB, 0x1C91CEC9, 0x8DC6F6F6, 0x3B193738, 0x938D7D61,
  0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFDBC29, 0x211BF8E9, 0x323BF6FD, 0xC4C971FD, 0x884943DB,
  0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFE89A2, 0x721D515E, 0x02D0A650, 0xB407A8AB, 0x84E45917,
  0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFF26BF, 0x614B35F7, 0xE678C631, 0xE67EFACD, 0x286FD64F,
  0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFCA48, 0x0C0BC045, 0x42E8F9F1, 0x742A203D, 0x0301253F,
  0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFF358, 0x5EE51139, 0x80C16668, 0x3019C88A, 0x77A1ACFF,
  0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFC6D, 0x3F5E4E54, 0xAB2F696F, 0x4D52A727, 0xAFCB63FF,
  0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFF8E, 0x553F9533, 0x994F30CF, 0x299CCA9F, 0xCAA71FFF,
  0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFF0, 0x66554CF0, 0x78DA4025, 0xB1E0F32A, 0xA660FFFF,
  0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x879996A5, 0x5293801C, 0x94AA5699, 0x9E1FFFFF,
  0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xF81E18C6, 0x631C0003, 0x8C663187, 0x81FFFFFF,
  0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFE01F07, 0x83E00000, 0x7C1E0F80, 0x7FFFFFFF,
  0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFE007, 0xFC000000, 0x03FE007F, 0xFFFFFFFF,
  0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFF8, 0x00000000, 0x0001FFFF, 0xFFFFFFFF,
  0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

#define MIC_ARRAY_48K_STAGE_2_TAP_COUNT 96
static constexpr right_shift_t stage2_48k_shift = 3;

static const int32_t WORD_ALIGNED stage2_48k_coefs[MIC_ARRAY_48K_STAGE_2_TAP_COUNT] =
{
    -0x2b915, -0x68daa, 0x12b1b, 0xe0dd5, 0x7aab9, -0x138439, -0x19f6aa, 0xe98af, 0x325327, 0x9d62c, -0x453461,
    -0x39da72, 0x3ff003, 0x79a63a, -0xf0b09, -0xb15cab, -0x56c4c6, 0xbb8595, 0xe472b5, -0x707afa, -0x16f40d3,
    -0x467c2b, 0x1b26780, 0x15a2769, -0x1613820, -0x28784d8, 0x45cf09, 0x35a69c3, 0x19be171, -0x345fdf3, -0x3eb6280,
    0x1d11f71, 0x5f64572, 0x1337b74, -0x6c761d0, -0x57cf5b3, 0x5581126, 0xa2a12d7, -0xcc4e19, -0xdbd10c4, -0x761b6b2,
    0xe0b081c, 0x13711ab2, -0x7854251, -0x23f260e4, -0xfcc09a7, 0x3a64a62b, 0x7fffffff, 0x7fffffff, 0x3a64a62b,
    -0xfcc09a7, -0x23f260e4, -0x7854251, 0x13711ab2, 0xe0b081c, -0x761b6b2, -0xdbd10c4, -0xcc4e19, 0xa2a12d7, 0x5581126,
    -0x57cf5b3, -0x6c761d0, 0x1337b74, 0x5f64572, 0x1d11f71, -0x3eb6280, -0x345fdf3, 0x19be171, 0x35a69c3, 0x45cf09,
    -0x28784d8, -0x1613820, 0x15a2769, 0x1b26780, -0x467c2b, -0x16f40d3, -0x707afa, 0xe472b5, 0xbb8595, -0x56c4c6,
    -0xb15cab, -0xf0b09, 0x79a63a, 0x3ff003, -0x39da72, -0x453461, 0x9d62c, 0x325327, 0xe98af, -0x19f6aa,
    -0x138439, 0x7aab9, 0xe0dd5, 0x12b1b, -0x68daa, -0x2b915
};
constexpr int mic_count = MIC_ARRAY_CONFIG_MIC_COUNT;
constexpr int decimation_factor = 2;


using TMicArray = mic_array::MicArray<mic_count,
                          mic_array::TwoStageDecimator<mic_count,
                                                       decimation_factor,
                                                       MIC_ARRAY_48K_STAGE_2_TAP_COUNT>,
                          mic_array::StandardPdmRxService<MIC_ARRAY_CONFIG_MIC_COUNT,
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


TMicArray mics_custom;


void ma_init()
{
  mics_custom.Decimator.Init(stage1_48k_coefs, stage2_48k_coefs, stage2_48k_shift);
  mics_custom.PdmRx.Init(pdm_res_custom.p_pdm_mics);

  mic_array_resources_configure(&pdm_res_custom, MIC_ARRAY_CONFIG_MCLK_DIVIDER);
  mic_array_pdm_clock_start(&pdm_res_custom);
}


void ma_task(
    chanend_t c_frames_out)
{
  mics_custom.OutputHandler.FrameTx.SetChannel(c_frames_out);

  mics_custom.PdmRx.InstallISR();
  mics_custom.PdmRx.UnmaskISR();

  mics_custom.ThreadEntry();
}
