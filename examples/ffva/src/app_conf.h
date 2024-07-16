// Copyright 2022-2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef APP_CONF_H_
#define APP_CONF_H_

/**
 * Application version numbers
 * These values can be read by the xvf_dfu host app
 * The xvf_dfu app is used with the FFVA-INT device only
 */
#ifndef APP_VERSION_MAJOR
#define APP_VERSION_MAJOR   255
#endif
#ifndef APP_VERSION_MINOR
#define APP_VERSION_MINOR   254
#endif
#ifndef APP_VERSION_PATCH
#define APP_VERSION_PATCH   253
#endif

/* Intertile port settings */
#define appconfUSB_AUDIO_PORT          0
#define appconfGPIO_T0_RPC_PORT        1
#define appconfGPIO_T1_RPC_PORT        2
#define appconfDEVICE_CONTROL_USB_PORT 3
#define appconfDEVICE_CONTROL_I2C_PORT 4
#define appconfSPI_AUDIO_PORT          5
#define appconfWW_SAMPLES_PORT         6
#define appconfAUDIOPIPELINE_PORT      7
#define appconfI2S_OUTPUT_SLAVE_PORT   8

#ifndef appconfINTENT_ENGINE_READY_SYNC_PORT
#define appconfINTENT_ENGINE_READY_SYNC_PORT      18
#endif /* appconfINTENT_ENGINE_READY_SYNC_PORT */

/* Application tile specifiers */
#include "platform/driver_instances.h"
#define ASR_TILE_NO                     FLASH_TILE_NO
#define FS_TILE_NO                      FLASH_TILE_NO
#define AUDIO_PIPELINE_OUTPUT_TILE_NO   FLASH_TILE_NO

/* Audio Pipeline Configuration */
#define appconfAUDIO_CLOCK_FREQUENCY            MIC_ARRAY_CONFIG_MCLK_FREQ
#define appconfPDM_CLOCK_FREQUENCY              MIC_ARRAY_CONFIG_PDM_FREQ
#define appconfAUDIO_PIPELINE_SAMPLE_RATE       16000
#define appconfAUDIO_PIPELINE_CHANNELS          MIC_ARRAY_CONFIG_MIC_COUNT
/* If in channel sample format, appconfAUDIO_PIPELINE_FRAME_ADVANCE == MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME*/
#define appconfAUDIO_PIPELINE_FRAME_ADVANCE     MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME

/* Enable audio response output */
#ifndef appconfAUDIO_PLAYBACK_ENABLED
#define appconfAUDIO_PLAYBACK_ENABLED           0
#endif

/* Intent Engine Configuration */
#define appconfINTENT_FRAME_BUFFER_MULT      (8*2)       /* total buffer size is this value * MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME */
#define appconfINTENT_SAMPLE_BLOCK_LENGTH    240

/* Maximum delay between a wake up phrase and command phrase */
#ifndef appconfINTENT_RESET_DELAY_MS
#if appconfAUDIO_PLAYBACK_ENABLED
#define appconfINTENT_RESET_DELAY_MS         5000
#else
#define appconfINTENT_RESET_DELAY_MS         4000
#endif
#endif

/* Output raw inferences, if set to 0, a state machine requires a wake up phrase
 * before a command phrase */
#ifndef appconfINTENT_RAW_OUTPUT
#define appconfINTENT_RAW_OUTPUT   0
#endif

/* Maximum number of detected intents to hold */
#ifndef appconfINTENT_QUEUE_LEN
#define appconfINTENT_QUEUE_LEN     10
#endif

/* External wakeup pin edge on intent found.  0 for rising edge, 1 for falling edge */
#ifndef appconfINTENT_WAKEUP_EDGE_TYPE
#define appconfINTENT_WAKEUP_EDGE_TYPE     0
#endif

/* Delay between external wakeup pin edge and intent output */
#ifndef appconfINTENT_TRANSPORT_DELAY_MS
#define appconfINTENT_TRANSPORT_DELAY_MS     50
#endif

#ifndef appconfINTENT_I2C_OUTPUT_ENABLED
#define appconfINTENT_I2C_OUTPUT_ENABLED   0
#endif

#ifndef appconfINTENT_I2C_OUTPUT_DEVICE_ADDR
#define appconfINTENT_I2C_OUTPUT_DEVICE_ADDR 0x01
#endif

#ifndef appconfINTENT_UART_OUTPUT_ENABLED
#define appconfINTENT_UART_OUTPUT_ENABLED   1
#endif

/**
 * if this flag is enabled, we get in the Cyberon engine a lot of errors like the following:
 * 'lost local output samples for intent'
 * TODO: Enable this flag
*/
//#ifndef UART_DUMP_RECORD
//#define UART_DUMP_RECORD 1
//#endif

#ifndef appconfUART_BAUD_RATE
#define appconfUART_BAUD_RATE       9600
#endif

/**
 * A positive delay will delay mics
 * A negative delay will delay ref
 */
#define appconfINPUT_SAMPLES_MIC_DELAY_MS        0

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

#ifndef appconfI2S_ENABLED
#define appconfI2S_ENABLED         1
#endif

#ifndef appconfUSB_ENABLED
#define appconfUSB_ENABLED         0
#endif

#ifndef appconfUSB_AUDIO_SAMPLE_RATE
#define appconfUSB_AUDIO_SAMPLE_RATE appconfAUDIO_PIPELINE_SAMPLE_RATE
#endif

#ifndef appconfSPI_OUTPUT_ENABLED
#define appconfSPI_OUTPUT_ENABLED  0
#endif

#ifndef appconfI2S_AUDIO_SAMPLE_RATE
#define appconfI2S_AUDIO_SAMPLE_RATE appconfAUDIO_PIPELINE_SAMPLE_RATE
#endif

#ifndef appconfEXTERNAL_MCLK
#define appconfEXTERNAL_MCLK       0
#endif

/*
 * This option sends all 6 16 KHz channels (two channels of processed audio,
 * stereo reference audio, and stereo microphone audio) out over a single
 * 48 KHz I2S line.
 */
#ifndef appconfI2S_TDM_ENABLED
#define appconfI2S_TDM_ENABLED     0
#endif

#define appconfI2S_MODE_MASTER     0
#define appconfI2S_MODE_SLAVE      1
#ifndef appconfI2S_MODE
#define appconfI2S_MODE            appconfI2S_MODE_MASTER
#endif


/**
 * Use I2S input instead of PDM microphones
 */
#ifndef appconfUSE_I2S_INPUT
#define appconfUSE_I2S_INPUT       0
#endif

#define appconfAEC_REF_USB         0
#define appconfAEC_REF_I2S         1
#ifndef appconfAEC_REF_DEFAULT
#define appconfAEC_REF_DEFAULT     appconfAEC_REF_I2S
#endif

#define appconfMIC_SRC_MICS        0
#define appconfMIC_SRC_USB         1
#ifndef appconfMIC_SRC_DEFAULT
#define appconfMIC_SRC_DEFAULT     appconfMIC_SRC_MICS
#endif

#define appconfUSB_AUDIO_RELEASE   0
#define appconfUSB_AUDIO_TESTING   1
#ifndef appconfUSB_AUDIO_MODE
#define appconfUSB_AUDIO_MODE      appconfUSB_AUDIO_RELEASE
#endif

#define appconfSPI_AUDIO_RELEASE   0
#define appconfSPI_AUDIO_TESTING   1
#ifndef appconfSPI_AUDIO_MODE
#define appconfSPI_AUDIO_MODE      appconfSPI_AUDIO_TESTING
#endif


#include "app_conf_check.h"

/* I/O and interrupt cores for Tile 0 */
/* Note, USB and SPI are mutually exclusive */
#define appconfXUD_IO_CORE                      1 /* Must be kept off core 0 with the RTOS tick ISR */
#define appconfSPI_IO_CORE                      1 /* Must be kept off core 0 with the RTOS tick ISR */
#define appconfUSB_INTERRUPT_CORE               2 /* Must be kept off I/O cores. Best kept off core 0 with the tick ISR. */
#define appconfUSB_SOF_INTERRUPT_CORE           3 /* Must be kept off I/O cores. Best kept off cores with other ISRs. */
#define appconfSPI_INTERRUPT_CORE               2 /* Must be kept off I/O cores. */

/* I/O and interrupt cores for Tile 1 */
#define appconfPDM_MIC_IO_CORE                  1 /* Must be kept off core 0 with the RTOS tick ISR */
#define appconfI2S_IO_CORE                      2 /* Must be kept off core 0 with the RTOS tick ISR */
#define appconfI2C_IO_CORE                      5 /* Must be kept off core 0 with the RTOS tick ISR */
#define appconfPDM_MIC_INTERRUPT_CORE           4 /* Must be kept off I/O cores. Best kept off core 0 with the tick ISR. */
#define appconfI2S_INTERRUPT_CORE               5 /* Must be kept off I/O cores. Best kept off core 0 with the tick ISR. */
#define appconfI2C_INTERRUPT_CORE               4 /* Must be kept off I/O cores. */

/* Task Priorities */
#define appconfSTARTUP_TASK_PRIORITY              (configMAX_PRIORITIES/2 + 5)
#define appconfAUDIO_PIPELINE_TASK_PRIORITY       (configMAX_PRIORITIES / 2)
#define appconfGPIO_RPC_HOST_PRIORITY             (configMAX_PRIORITIES/2 + 2)
#define appconfGPIO_TASK_PRIORITY                 (configMAX_PRIORITIES/2 + 2)
#define appconfI2C_TASK_PRIORITY                  (configMAX_PRIORITIES/2 + 2)
#define appconfUSB_MGR_TASK_PRIORITY              (configMAX_PRIORITIES/2 + 1)
#define appconfUSB_AUDIO_TASK_PRIORITY            (configMAX_PRIORITIES/2 + 1)
#define appconfSPI_TASK_PRIORITY                  (configMAX_PRIORITIES/2 + 1)
#define appconfQSPI_FLASH_TASK_PRIORITY           (configMAX_PRIORITIES/2 + 0)
#define appconfINTENT_MODEL_RUNNER_TASK_PRIORITY  (configMAX_PRIORITIES - 2)
#define appconfLED_TASK_PRIORITY                  (configMAX_PRIORITIES / 2 - 1)

/* Software PLL settings for mclk recovery configurations */
/* see fractions.h and register_setup.h for other pll settings */
#define appconfLRCLK_NOMINAL_HZ     appconfI2S_AUDIO_SAMPLE_RATE
#define appconfBCLK_NOMINAL_HZ      (appconfLRCLK_NOMINAL_HZ * 64)
#define PLL_RATIO                   (MIC_ARRAY_CONFIG_MCLK_FREQ / appconfLRCLK_NOMINAL_HZ)
#define PLL_CONTROL_LOOP_COUNT_INT  512  // How many refclk ticks (LRCLK) per control loop iteration. Aim for ~100Hz
#define PLL_PPM_RANGE               1000 // Max allowable diff in clk count. For the PID constants we
                                         // have chosen, this number should be larger than the number
                                         // of elements in the look up table as the clk count diff is
                                         // added to the LUT index with a multiplier of 1. Only used for INT mclkless


#endif /* APP_CONF_H_ */
