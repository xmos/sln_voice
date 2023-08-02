// Copyright (c) 2020-2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#include <platform.h>
#include <xs1.h>
#include <xcore/channel.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"
#include "queue.h"

/* Library headers */
#include "rtos_printf.h"
#include "src.h"

/* App headers */
#include "app_conf.h"
#include "platform/platform_init.h"
#include "platform/driver_instances.h"
#include "usb_support.h"
#include "usb_audio.h"
#include "ww_model_runner/ww_model_runner.h"
#include "fs_support.h"

#include "gpio_test/gpio_test.h"
#include "i2s_audio.h"

volatile int mic_from_usb = appconfMIC_SRC_DEFAULT;
volatile int aec_ref_source = appconfAEC_REF_DEFAULT;

void vApplicationMallocFailedHook(void)
{
    rtos_printf("Malloc Failed on tile %d!\n", THIS_XCORE_TILE);
    xassert(0);
    for(;;);
}

static void mem_analysis(void)
{
	for (;;) {
		rtos_printf("Tile[%d]:\n\tMinimum heap free: %d\n\tCurrent heap free: %d\n", THIS_XCORE_TILE, xPortGetMinimumEverFreeHeapSize(), xPortGetFreeHeapSize());
		vTaskDelay(pdMS_TO_TICKS(5000));
	}
}

static void usb_to_i2s_slave_intertile(void *args) {
    (void) args;
    int32_t *usb_to_i2s_samps;
    for(;;)
    {
        unsigned num_samps = usb_audio_recv(intertile_usb_audio_ctx,
                                            &usb_to_i2s_samps
                                        );
        if(num_samps)
        {
            rtos_i2s_tx(i2s_ctx,
                (int32_t*) usb_to_i2s_samps,
                num_samps,
                portMAX_DELAY);
        }
    }
}

static void i2s_slave_to_usb_intertile(void *args) {
    (void) args;
    int32_t i2s_to_usb_samps_interleaved[240*2][2]; // TODO fix all the hardcodings

    for(;;)
    {
        size_t bytes_received;

        bytes_received = rtos_intertile_rx_len(
                intertile_ctx,
                appconfAUDIOPIPELINE_PORT,
                portMAX_DELAY);

        if (bytes_received > 0) {
            xassert(bytes_received <= sizeof(i2s_to_usb_samps_interleaved));

            rtos_intertile_rx_data(
                    intertile_ctx,
                    i2s_to_usb_samps_interleaved,
                    bytes_received);

            usb_audio_send(i2s_to_usb_samps_interleaved, (bytes_received >> 3), 2);
        }

    }
}

void startup_task(void *arg)
{
    rtos_printf("Startup task running from tile %d on core %d\n", THIS_XCORE_TILE, portGET_CORE_ID());

    platform_start();

#if ON_TILE(1) && appconfI2S_ENABLED
    xTaskCreate((TaskFunction_t) usb_to_i2s_slave_intertile,
                "usb_to_i2s_slave_intertile",
                RTOS_THREAD_STACK_SIZE(usb_to_i2s_slave_intertile),
                NULL,
                appconfAUDIO_PIPELINE_TASK_PRIORITY,
                NULL);
#endif

#if ON_TILE(0) && appconfUSB_ENABLED
    xTaskCreate((TaskFunction_t) i2s_slave_to_usb_intertile,
                "i2s_slave_to_usb_intertile",
                RTOS_THREAD_STACK_SIZE(i2s_slave_to_usb_intertile),
                NULL,
                appconfAUDIO_PIPELINE_TASK_PRIORITY,
                NULL);
#endif


#if ON_TILE(1)
    gpio_test(gpio_ctx_t0);
#endif

#if ON_TILE(1)
    i2s_audio_recv_init();
#endif

#if ON_TILE(FS_TILE_NO)
    rtos_fatfs_init(qspi_flash_ctx);
    rtos_dfu_image_print_debug(dfu_image_ctx);
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
    usb_audio_init(intertile_usb_audio_ctx, appconfUSB_AUDIO_TASK_PRIORITY);
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
