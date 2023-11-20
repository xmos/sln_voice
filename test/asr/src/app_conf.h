// Copyright 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef APP_CONF_H_
#define APP_CONF_H_

/* Intertile port settings */
#define appconfXSCOPE_FILEIO_PORT               0
#define appconfXSCOPE_FILEIO_READY_SYNC_PORT    1

/* Application tile specifiers */
#include "platform/driver_instances.h"

#ifndef appconfASR_BRICK_SIZE_SAMPLES
#define appconfASR_BRICK_SIZE_SAMPLES                 240  // typically set in asr.cmake
#endif 

#ifndef appconfASR_MISSING_METADATA_CORRECTION  
#define appconfASR_MISSING_METADATA_CORRECTION        (40 * appconfASR_BRICK_SIZE_SAMPLES)
#endif

#define appconfINPUT_FILENAME                   "input.wav\0"
#define appconfOUTPUT_FILENAME                  "output.log\0"
#define appconfINPUT_CHANNELS                   1
#define appconfSAMPLE_BIT_DEPTH                 32
#define appconfSAMPLE_SIZE_BYTES                (appconfSAMPLE_BIT_DEPTH / 8)
#define appconfASR_BRICK_SIZE_BYTES             (appconfASR_BRICK_SIZE_SAMPLES * appconfINPUT_CHANNELS * appconfSAMPLE_SIZE_BYTES)

/* Task Priorities */
#define appconfSTARTUP_TASK_PRIORITY            (configMAX_PRIORITIES / 2)
#define appconfXSCOPE_IO_TASK_PRIORITY          (configMAX_PRIORITIES - 1)
#define appconfQSPI_FLASH_TASK_PRIORITY         (configMAX_PRIORITIES - 1)

#endif /* APP_CONF_H_ */
