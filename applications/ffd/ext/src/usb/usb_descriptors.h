// Copyright (c) 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef USB_DESCRIPTORS_H_
#define USB_DESCRIPTORS_H_

#include "tusb_config.h"

#if appconfUSB_AUDIO_ENABLED
#if CFG_TUD_AUDIO_ENABLE_EP_IN && CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX > 0
#define AUDIO_INPUT_ENABLED 1
#else
#define AUDIO_INPUT_ENABLED 0
#endif
#if CFG_TUD_AUDIO_ENABLE_EP_OUT && CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX > 0
#define AUDIO_OUTPUT_ENABLED 1
#else
#define AUDIO_OUTPUT_ENABLED 0
#endif
#else
#define AUDIO_INPUT_ENABLED 0
#define AUDIO_OUTPUT_ENABLED 0
#endif /* appconfUSB_AUDIO_ENABLED */

enum {
#if appconfUSB_AUDIO_ENABLED
    ITF_NUM_AUDIO_CONTROL = 0,
#if AUDIO_OUTPUT_ENABLED
    ITF_NUM_AUDIO_STREAMING_SPK,
#endif
#if AUDIO_INPUT_ENABLED
    ITF_NUM_AUDIO_STREAMING_MIC,
#endif
#endif
#if appconfINFERENCE_USB_OUTPUT_ENABLED
    ITF_NUM_KEYWORD,
#endif
    ITF_NUM_TOTAL
};

// Unit numbers are arbitrary selected
#define UAC2_ENTITY_CLOCK               0x01
// Speaker path
#define UAC2_ENTITY_SPK_INPUT_TERMINAL  0x11
#define UAC2_ENTITY_SPK_FEATURE_UNIT    0x12
#define UAC2_ENTITY_SPK_OUTPUT_TERMINAL 0x13

// Microphone path
#define UAC2_ENTITY_MIC_INPUT_TERMINAL  0x21
#define UAC2_ENTITY_MIC_FEATURE_UNIT    0x22
#define UAC2_ENTITY_MIC_OUTPUT_TERMINAL 0x23

#define VENDOR_EP_SIZE  512

#endif /* USB_DESCRIPTORS_H_ */
