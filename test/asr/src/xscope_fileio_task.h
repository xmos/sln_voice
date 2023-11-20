// Copyright 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef XSCOPE_FILEIO_TASK_H_
#define XSCOPE_FILEIO_TASK_H_

#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"

extern QueueHandle_t tx_to_host_queue;
extern QueueHandle_t rx_from_host_queue;

void xscope_fileio_tasks_create(unsigned priority, void* app_data);

/* Signal to fileio that the application is done and files can be closed */
void xscope_fileio_user_done(void);

size_t tx_to_host(uint8_t *buf, size_t size_bytes);
size_t rx_from_host(int8_t **buf, size_t size_bytes);

size_t tx_to_audio_pipeline(uint8_t *buf, size_t size_bytes);
size_t rx_from_audio_pipeline(uint8_t **buf, size_t size_bytes);

#endif /* XSCOPE_FILEIO_TASK_H_ */
