// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#include "FreeRTOS.h"
#include "task.h"

#include "app_conf.h"
#include "platform/driver_instances.h"
#include "xscope_fileio_task.h"

size_t tx_to_host(uint8_t *buf, size_t size_bytes) {
    rtos_intertile_tx(intertile_ctx,
                    appconfXSCOPE_FILEIO_PORT,
                    buf,
                    size_bytes);

    return size_bytes;
}

size_t rx_from_host(int8_t **buf, size_t size_bytes) {
    size_t bytes_received = 0;
    bytes_received = rtos_intertile_rx_len(
            intertile_ctx,
            appconfXSCOPE_FILEIO_PORT,
            portMAX_DELAY);
    
    xassert(bytes_received == size_bytes);

    rtos_intertile_rx_data(
            intertile_ctx,
            buf,
            bytes_received);

    return bytes_received;
}

size_t tx_to_audio_pipeline(uint8_t *buf, size_t size_bytes) {
    rtos_intertile_tx(intertile_ctx,
            appconfXSCOPE_FILEIO_PORT,
            buf,
            size_bytes);

    return size_bytes;
}

size_t rx_from_audio_pipeline(uint8_t **buf, size_t size_bytes) {
    size_t bytes_received = 0;
    bytes_received = rtos_intertile_rx_len(
            intertile_ctx,
            appconfXSCOPE_FILEIO_PORT,
            portMAX_DELAY);
    rtos_intertile_rx_data(
            intertile_ctx,
            buf,
            bytes_received);
    
    return bytes_received;
}
