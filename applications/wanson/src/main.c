// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

/* System headers */
#include <platform.h>
#include <xs1.h>
#include <xcore/channel.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"
#include "queue.h"
#include "event_groups.h"

/* Library headers */
#include "rtos_printf.h"
#include "src.h"

/* App headers */
#include "app_conf.h"
#include "platform/platform_init.h"
#include "platform/driver_instances.h"
#include "usb_support.h"
#include "usb_audio.h"
#include "audio_pipeline/audio_pipeline.h"
#include "inference_engine.h"
#include "fs_support.h"
#include "gpio_ctrl/gpi_ctrl.h"
// #include "keyword_inference.h"
#include "inference_hmi/inference_hmi.h"
#include "rtos_swmem.h"

volatile int mic_from_usb = appconfMIC_SRC_DEFAULT;

#define IS_RAM(a)                    \
  (((uintptr_t)a >= XS1_RAM_BASE) && \
   ((uintptr_t)a <= (XS1_RAM_BASE + XS1_RAM_SIZE)))

#define IS_SWMEM(a)                    \
  (((uintptr_t)a >= XS1_SWMEM_BASE) && \
   (((uintptr_t)a <= (XS1_SWMEM_BASE - 1 + XS1_SWMEM_SIZE))))

static int cnt = 0;
/* Uh oh, hacking this in for now */
size_t swmem_load(void *dest, const void *src, size_t size)
{
    // xassert(IS_SWMEM(src));
    rtos_printf("cnt: %d\n", cnt++);

    if (IS_SWMEM(src)) {
        if (qspi_flash_ctx != NULL) {
            rtos_printf(
                    "BEFORE rtos_qspi_flash_read   dest=0x%x    src=0x%x    size=%d\n",
                    dest, src, size);
            uint32_t tic = get_reference_time();

            rtos_qspi_flash_read(qspi_flash_ctx, (uint8_t *)dest,
                                 (unsigned)(src - XS1_SWMEM_BASE), size);
            rtos_printf("AFTER rtos_qspi_flash_read   size=%d      duration=%lu\n",
                        size,
                        (get_reference_time() - tic) / PLATFORM_REFERENCE_MHZ);
            return size;
        }
    } else if (IS_RAM(src)) {
        memcpy(dest,src,size);
    } else {
        rtos_printf("***not expected wanted src=0x%x    size=%d\n", src, size);
        // xassert(0);
    }
    return 0;
}

static char uart_buffer[100] = {0};
static int ndx = 0;
void uart_write(char data) //API for Wanson's Debug
{
    if( data == '\n') {
        // rtos_printf("uart:%s\n", uart_buffer);

#if 0
        for(int i=0; i<ndx; i++) {
            rtos_printf("%x",uart_buffer[i]);
        }
        rtos_printf("\n");
#endif
        ndx = 0;

    } else {
        uart_buffer[ndx] = data;
        ndx++;
    }
// char data_buf=data;
// rtos_uart_write(uart_ctx, &data_buf,1);
}

void audio_pipeline_input(void *input_app_data,
                          int32_t **input_audio_frames,
                          size_t ch_count,
                          size_t frame_count)
{
    (void) input_app_data;

#if appconfUSB_ENABLED
    int32_t **usb_mic_audio_frame = NULL;

    if (mic_from_usb) {
        usb_mic_audio_frame = input_audio_frames;
        usb_audio_recv(intertile_ctx,
                       frame_count,
                       usb_mic_audio_frame,
                       ch_count);
    } else {
        rtos_mic_array_rx(mic_array_ctx,
                          input_audio_frames,
                          frame_count,
                          portMAX_DELAY);
    }
#else
    static int flushed;
    while (!flushed) {
        size_t received;
        received = rtos_mic_array_rx(mic_array_ctx,
                                     input_audio_frames,
                                     frame_count,
                                     0);
        if (received == 0) {
            rtos_mic_array_rx(mic_array_ctx,
                              input_audio_frames,
                              frame_count,
                              portMAX_DELAY);
            flushed = 1;
        }
    }

    rtos_mic_array_rx(mic_array_ctx,
                      input_audio_frames,
                      frame_count,
                      portMAX_DELAY);
#endif
}

int audio_pipeline_output(void *output_app_data,
                          int32_t **output_audio_frames,
                          size_t ch_count,
                          size_t frame_count)
{
    (void) output_app_data;

#if appconfI2S_ENABLED
    /* I2S expects sample channel format */
    int32_t tmp[appconfAUDIO_PIPELINE_FRAME_ADVANCE][appconfAUDIO_PIPELINE_CHANNELS];
    for (int j=0; j<appconfAUDIO_PIPELINE_FRAME_ADVANCE; j++) {
        /* ASR output is first */
        tmp[j][0] = output_audio_frames[j];
        tmp[j][1] = output_audio_frames[j];
    }

    rtos_i2s_tx(i2s_ctx,
                (int32_t*) tmp,
                frame_count,
                portMAX_DELAY);
#endif

#if appconfUSB_ENABLED
    usb_audio_send(intertile_ctx,
                frame_count,
                output_audio_frames,
                4);
#endif

#if appconfINFERENCE_ENABLED
    inference_engine_sample_push(output_audio_frames, frame_count);
#endif

    return AUDIO_PIPELINE_FREE_FRAME;
}

void vApplicationMallocFailedHook(void)
{
    rtos_printf("Malloc Failed on tile %d!\n", THIS_XCORE_TILE);
    xassert(0);
    for(;;);
}

static void mem_analysis(void)
{
	for (;;) {
		// rtos_printf("Tile[%d]:\n\tMinimum heap free: %d\n\tCurrent heap free: %d\n", THIS_XCORE_TILE, xPortGetMinimumEverFreeHeapSize(), xPortGetFreeHeapSize());
		vTaskDelay(pdMS_TO_TICKS(5000));
	}
}

void startup_task(void *arg)
{
    rtos_printf("Startup task running from tile %d on core %d\n", THIS_XCORE_TILE, portGET_CORE_ID());

    platform_start();

#if ON_TILE(1)
    gpio_gpi_init(gpio_ctx_t0);
#endif

#if ON_TILE(FS_TILE_NO)
    rtos_swmem_init(RTOS_SWMEM_READ_FLAG);
    rtos_swmem_start(configMAX_PRIORITIES);
    // rtos_fatfs_init(qspi_flash_ctx);
#endif

#if appconfINFERENCE_ENABLED && ON_TILE(INFERENCE_TILE_NO)
    // keyword_engine_args_t *kw_eng_args = pvPortMalloc( sizeof(keyword_engine_args_t) );
    // kw_eng_args->egrp_inference = xEventGroupCreate();
    inference_engine_create(appconfINFERENCE_MODEL_RUNNER_TASK_PRIORITY, NULL);
    // inference_hmi_create(appconfINFERENCE_HMI_TASK_PRIORITY, kw_eng_args->egrp_inference);
#endif

#if ON_TILE(AUDIO_PIPELINE_TILE_NO)
    audio_pipeline_init(NULL, NULL);
#endif

    mem_analysis();
    /*
     * TODO: Watchdog?
     */
}

void vApplicationMinimalIdleHook(void)
{
    rtos_printf("idle hook on tile %d core %d\n", THIS_XCORE_TILE, rtos_core_id_get());
    asm volatile("waiteu");
}

static void tile_common_init(chanend_t c)
{
    platform_init(c);
    chanend_free(c);

#if appconfUSB_ENABLED && ON_TILE(USB_TILE_NO)
    usb_audio_init(intertile_ctx, appconfUSB_AUDIO_TASK_PRIORITY);
#endif

    xTaskCreate((TaskFunction_t) startup_task,
                "startup_task",
                RTOS_THREAD_STACK_SIZE(startup_task),
                NULL,
                appconfSTARTUP_TASK_PRIORITY,
                NULL);

    rtos_printf("start scheduler on tile %d\n", THIS_XCORE_TILE);
    vTaskStartScheduler();
}

#if ON_TILE(0)
void main_tile0(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
{
    (void) c0;
    (void) c2;
    (void) c3;

    tile_common_init(c1);
}
#endif

#if ON_TILE(1)
void main_tile1(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
{
    (void) c1;
    (void) c2;
    (void) c3;

    tile_common_init(c0);
}
#endif
