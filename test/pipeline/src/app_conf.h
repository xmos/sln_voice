// Copyright 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef APP_CONF_H_
#define APP_CONF_H_

/* Intertile port settings */
#define appconfAUDIOPIPELINE_PORT               0
#define appconfXSCOPE_FILEIO_AUDIO_PORT         1
#define appconfXSCOPE_FILEIO_TRACE_PORT         2
#define appconfXSCOPE_FILEIO_READY_SYNC_PORT    3

/* Application tile specifiers */
#include "platform/driver_instances.h"

/* Audio Pipeline Configuration */
#define appconfAUDIO_PIPELINE_SAMPLE_RATE       16000
#define appconfAUDIO_PIPELINE_CHANNELS          2
#define appconfAUDIO_PIPELINE_FRAME_ADVANCE     240

#ifndef appconfAUDIO_PIPELINE_INPUT_CHANNELS
#define appconfAUDIO_PIPELINE_INPUT_CHANNELS    4
#endif

#ifndef appconfAUDIO_PIPELINE_INPUT_TILE_NO
#define appconfAUDIO_PIPELINE_INPUT_TILE_NO     0
#endif

#ifndef appconfAUDIO_PIPELINE_OUTPUT_TILE_NO
#define appconfAUDIO_PIPELINE_OUTPUT_TILE_NO    0
#endif

#ifndef appconfAUDIO_PIPELINE_SUPPORTS_TRACE
#define appconfAUDIO_PIPELINE_SUPPORTS_TRACE    0
#endif

#ifdef appconfPIPELINE_BYPASS
#define appconfAUDIO_PIPELINE_SKIP_STATIC_DELAY  1
#define appconfAUDIO_PIPELINE_SKIP_AEC           1
#define appconfAUDIO_PIPELINE_SKIP_IC_AND_VNR    1
#define appconfAUDIO_PIPELINE_SKIP_NS            1
#define appconfAUDIO_PIPELINE_SKIP_AGC           1
#endif

#ifndef appconfAUDIO_PIPELINE_SKIP_STATIC_DELAY
#define appconfAUDIO_PIPELINE_SKIP_STATIC_DELAY  0
#endif

#ifndef appconfAUDIO_PIPELINE_SKIP_AEC
#define appconfAUDIO_PIPELINE_SKIP_AEC           0
#endif

#ifndef appconfAUDIO_PIPELINE_SKIP_IC_AND_VNR
#define appconfAUDIO_PIPELINE_SKIP_IC_AND_VNR    0
#endif

#ifndef appconfAUDIO_PIPELINE_SKIP_NS
#define appconfAUDIO_PIPELINE_SKIP_NS            0
#endif

#ifndef appconfAUDIO_PIPELINE_SKIP_AGC
#define appconfAUDIO_PIPELINE_SKIP_AGC           0
#endif

/* xscope_fileio Configuration */
#define appconfSAMPLE_BIT_DEPTH                 32
#define appconfSAMPLE_SIZE_BYTES                (appconfSAMPLE_BIT_DEPTH / 8)

#define appconfINPUT_FILENAME                   "input.wav"
//#define appconfAUDIO_PIPELINE_INPUT_CHANNELS                 Set in pipeline.cmake
#define appconfINPUT_BRICK_SIZE_BYTES           (appconfAUDIO_PIPELINE_FRAME_ADVANCE * appconfAUDIO_PIPELINE_INPUT_CHANNELS * appconfSAMPLE_SIZE_BYTES)

#define appconfTRACE_FILENAME                   "output.log"
#define appconfOUTPUT_FILENAME                  "output.wav"
#define appconfOUTPUT_CHANNELS                  2
#define appconfOUTPUT_BRICK_SIZE_BYTES          (appconfAUDIO_PIPELINE_FRAME_ADVANCE * appconfOUTPUT_CHANNELS * appconfSAMPLE_SIZE_BYTES)
#define appconfOUTPUT_TRACE_SIZE_BYTES          (2048)

/* Task Priorities */
#define appconfSTARTUP_TASK_PRIORITY            (configMAX_PRIORITIES / 2)
#define appconfAUDIO_PIPELINE_TASK_PRIORITY     (configMAX_PRIORITIES - 1)
#define appconfXSCOPE_IO_TASK_PRIORITY          (configMAX_PRIORITIES - 1)


#endif /* APP_CONF_H_ */
