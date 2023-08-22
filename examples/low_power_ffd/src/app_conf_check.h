// Copyright 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef APP_CONF_CHECK_H_
#define APP_CONF_CHECK_H_

#if ASR_TILE_NO == AUDIO_PIPELINE_TILE_NO
#error "This application currently expects the ASR and audio pipeline to be on separate tiles."
#endif

#endif /* APP_CONF_CHECK_H_ */
