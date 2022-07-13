// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

/* STD headers */
#include <platform.h>
#include <xs1.h>
#include <xcore/hwtimer.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* App headers */
#include "app_conf.h"
#include "platform/driver_instances.h"
#include "intent_handler/intent_handler.h"
#include "fs_support.h"
#include "ff.h"

#define FILEPATH_WAKEUP		"wakeup.wav"
#define FILEPATH_310		"310.wav"

#define WAKEUP_LOW  (appconfINTENT_WAKEUP_EDGE_TYPE)
#define WAKEUP_HIGH (appconfINTENT_WAKEUP_EDGE_TYPE == 0)

#include "dr_wav_freertos_port.h"

static void proc_keyword_res(void *args) {
    QueueHandle_t q_intent = (QueueHandle_t) args;
    int32_t id = 0;
    int32_t host_status = 0;

    configASSERT(q_intent != 0);

    const rtos_gpio_port_id_t p_out_wakeup = rtos_gpio_port(XS1_PORT_1D);       /* PORT_SPI_MOSI on XK_VOICE_L71*/
    const rtos_gpio_port_id_t p_in_host_status = rtos_gpio_port(XS1_PORT_1P);   /* PORT_SPI_MISO on XK_VOICE_L71*/

    rtos_gpio_port_enable(gpio_ctx_t0, p_out_wakeup);
    rtos_gpio_port_enable(gpio_ctx_t0, p_in_host_status);

    rtos_gpio_port_out(gpio_ctx_t0, p_out_wakeup, WAKEUP_LOW);

    FIL file_wakeup;
    FIL file_310;
    FRESULT result;
    drwav wav_wakeup;
    drwav wav_310;
    size_t framesRead = 0;
    int32_t* file_audio = NULL;
    int32_t* wakeup_audio = NULL;

    result = f_open(&file_wakeup, FILEPATH_WAKEUP, FA_READ);
    result |= f_open(&file_310, FILEPATH_310, FA_READ);

    if (result == FR_OK) {
        drwav_init(
                &wav_wakeup,
                drwav_read_proc_port,
                drwav_seek_proc_port,
                &file_wakeup,
                &drwav_memory_cbs);
        drwav_init(
                &wav_310,
                drwav_read_proc_port,
                drwav_seek_proc_port,
                &file_310,
                &drwav_memory_cbs);

        file_audio = pvPortMalloc(appconfAUDIO_PIPELINE_FRAME_ADVANCE * sizeof(int32_t));
        wakeup_audio = pvPortMalloc(2*(appconfAUDIO_PIPELINE_FRAME_ADVANCE * sizeof(int32_t)));
    }

    while(1) {
        xQueueReceive(q_intent, &id, portMAX_DELAY);

        host_status = rtos_gpio_port_in(gpio_ctx_t0, p_in_host_status);

        if (host_status == 0) { /* Host is not awake */
            rtos_gpio_port_out(gpio_ctx_t0, p_out_wakeup, WAKEUP_HIGH);
            vTaskDelay(pdMS_TO_TICKS(appconfINTENT_TRANSPORT_DELAY_MS));
            rtos_gpio_port_out(gpio_ctx_t0, p_out_wakeup, WAKEUP_LOW);
        }
#if appconfINFERENCE_I2C_OUTPUT_ENABLED
        i2c_res_t ret;
        uint32_t buf = id;
        size_t sent = 0;

        ret = rtos_i2c_master_write(
            i2c_master_ctx,
            appconfINFERENCE_I2C_OUTPUT_DEVICE_ADDR,
            (uint8_t*)&buf,
            sizeof(uint32_t),
            &sent,
            1
        );

        if (ret != I2C_ACK) {
            rtos_printf("I2C inference output was not acknowledged\n\tSent %d bytes\n", sent);
        }
#endif
#if appconfINFERENCE_UART_OUTPUT_ENABLED && (UART_TILE_NO == INFERENCE_TILE_NO)
        uint32_t buf_uart = id;
        rtos_uart_tx_write(uart_tx_ctx, (uint8_t*)&buf_uart, sizeof(uint32_t));
#endif

        if ((file_audio != NULL) && (wakeup_audio != NULL)) {

            drwav wav = (id == 100) ? wav_wakeup : wav_310;
            /* TODO this will be encapulated into a play function */
            while(1) {
                memset(file_audio, 0x00, appconfAUDIO_PIPELINE_FRAME_ADVANCE*sizeof(int32_t));
                framesRead = drwav_read_pcm_frames(&wav, appconfAUDIO_PIPELINE_FRAME_ADVANCE, file_audio);
                memset(wakeup_audio, 0x00, 2*(appconfAUDIO_PIPELINE_FRAME_ADVANCE)*sizeof(int32_t));
                for (int i=0; i<framesRead; i++) {
                    wakeup_audio[(2*i)+0] = file_audio[i];
                    wakeup_audio[(2*i)+1] = file_audio[i];
                }

                rtos_i2s_tx(i2s_ctx,
                            (int32_t*) wakeup_audio,
                            appconfAUDIO_PIPELINE_FRAME_ADVANCE,
                            portMAX_DELAY);

                if (framesRead != appconfAUDIO_PIPELINE_FRAME_ADVANCE) {
                    drwav_seek_to_pcm_frame(&wav, 0);
                    break;
                }
            }
        }

    }
}

int32_t intent_handler_create(uint32_t priority, void *args)
{
    xTaskCreate((TaskFunction_t)proc_keyword_res,
                "proc_keyword_res",
                RTOS_THREAD_STACK_SIZE(proc_keyword_res),
                args,
                priority,
                NULL);

    return 0;
}
