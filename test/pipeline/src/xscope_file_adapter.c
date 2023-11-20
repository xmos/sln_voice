// Copyright 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "FreeRTOS.h"
#include "task.h"

#include "app_conf.h"
#include "platform/driver_instances.h"
#include "xscope_fileio_task.h"

//*****************************
// Audio file adapter functions
//*****************************

#if (appconfAUDIO_PIPELINE_OUTPUT_TILE_NO == 0)
size_t tx_audio_to_host(uint8_t *buf, size_t size_bytes) {
    xQueueSend(tx_audio_to_host_queue, buf, portMAX_DELAY);

    return size_bytes;
}
#elif (appconfAUDIO_PIPELINE_OUTPUT_TILE_NO == 1)
size_t tx_audio_to_host(uint8_t *buf, size_t size_bytes) {
    rtos_intertile_tx(intertile_ctx,
                    appconfXSCOPE_FILEIO_AUDIO_PORT,
                    buf,
                    size_bytes);

    return size_bytes;
}
#endif


#if (appconfAUDIO_PIPELINE_INPUT_TILE_NO == 0)
size_t rx_audio_from_host(int8_t **buf, size_t size_bytes) {
    xQueueReceive(rx_audio_from_host_queue, buf, portMAX_DELAY);

    return size_bytes;
}
#elif (appconfAUDIO_PIPELINE_INPUT_TILE_NO == 1)
size_t rx_audio_from_host(int8_t **buf, size_t size_bytes) {
    size_t bytes_received = 0;
    bytes_received = rtos_intertile_rx_len(
            intertile_ctx,
            appconfXSCOPE_FILEIO_AUDIO_PORT,
            portMAX_DELAY);
    
    xassert(bytes_received == size_bytes);

    rtos_intertile_rx_data(
            intertile_ctx,
            buf,
            bytes_received);

    return bytes_received;
}
#endif

#if (appconfAUDIO_PIPELINE_INPUT_TILE_NO == 0)
size_t tx_to_audio_pipeline(uint8_t *buf, size_t size_bytes) {
    rtos_printf("NOT IMPLEMENTED\n");
    xassert(1);
    return 0;
}
#elif (appconfAUDIO_PIPELINE_INPUT_TILE_NO == 1)
size_t tx_to_audio_pipeline(uint8_t *buf, size_t size_bytes) {
    rtos_intertile_tx(intertile_ctx,
            appconfXSCOPE_FILEIO_AUDIO_PORT,
            buf,
            size_bytes);

    return size_bytes;
}
#endif

#if (appconfAUDIO_PIPELINE_OUTPUT_TILE_NO == 0)
size_t rx_from_audio_pipeline(uint8_t **buf, size_t size_bytes) {
    xQueueReceive(tx_audio_to_host_queue, buf, portMAX_DELAY);

    return size_bytes;
}
#elif (appconfAUDIO_PIPELINE_OUTPUT_TILE_NO == 1)
size_t rx_from_audio_pipeline(uint8_t **buf, size_t size_bytes) {
    size_t bytes_received = 0;
    bytes_received = rtos_intertile_rx_len(
            intertile_ctx,
            appconfXSCOPE_FILEIO_AUDIO_PORT,
            portMAX_DELAY);
    rtos_intertile_rx_data(
            intertile_ctx,
            buf,
            bytes_received);
    
    return bytes_received;
}
#endif

//*****************************
// Trace file adapter functions
//*****************************
#if appconfAUDIO_PIPELINE_SUPPORTS_TRACE

#if (appconfAUDIO_PIPELINE_OUTPUT_TILE_NO == 0)
size_t tx_trace_to_host(int8_t *buf, size_t size_bytes) {
    xQueueSend(tx_trace_to_host_queue, buf, portMAX_DELAY);

    return size_bytes;
}
#elif (appconfAUDIO_PIPELINE_OUTPUT_TILE_NO == 1)
size_t tx_trace_to_host(int8_t *buf, size_t size_bytes) {
    rtos_intertile_tx(intertile_ctx,
                    appconfXSCOPE_FILEIO_TRACE_PORT,
                    buf,
                    size_bytes);

    return size_bytes;
}
#endif

#if (appconfAUDIO_PIPELINE_INPUT_TILE_NO == 0)
size_t rx_trace_from_app(int8_t **buf, size_t size_bytes) {
    xQueueReceive(rx_trace_from_app_queue, buf, portMAX_DELAY);

    return size_bytes;
}
#elif (appconfAUDIO_PIPELINE_INPUT_TILE_NO == 1)
size_t rx_trace_from_app(int8_t **buf, size_t size_bytes) {
    size_t bytes_received = 0;
    bytes_received = rtos_intertile_rx_len(
            intertile_ctx,
            appconfXSCOPE_FILEIO_TRACE_PORT,
            portMAX_DELAY);
    
    rtos_intertile_rx_data(
            intertile_ctx,
            buf,
            bytes_received);

    return bytes_received;
}
#endif

#endif // appconfAUDIO_PIPELINE_SUPPORTS_TRACE
