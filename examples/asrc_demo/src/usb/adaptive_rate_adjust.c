// Copyright 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#define DEBUG_UNIT ADAPTIVE_USB
#define DEBUG_PRINT_ENABLE_ADAPTIVE_USB 1

// Taken from usb_descriptors.c
#define USB_AUDIO_EP 0x01

#include "FreeRTOS.h"
#include "queue.h"

#include "adaptive_rate_adjust.h"
#include "adaptive_rate_callback.h"

#include <stdbool.h>
#include <xcore/port.h>
#include <rtos_printf.h>
#include <xscope.h>

#include <xcore/assert.h>
#include <xcore/triggerable.h>
#include <rtos_interrupt.h>

#include "stream_buffer.h"
#include "rate_server.h"
#include "tusb.h"

#ifndef USB_ADAPTIVE_TASK_PRIORITY
#define USB_ADAPTIVE_TASK_PRIORITY (configMAX_PRIORITIES-1)
#endif /* USB_ADAPTIVE_TASK_PRIORITY */

#define DATA_EVENT_QUEUE_SIZE 2

typedef struct usb_audio_rate_packet_desc {
    uint32_t cur_time;
    uint32_t ep_num;
    uint32_t ep_dir;
    size_t xfer_len;
} usb_audio_rate_packet_desc_t;

static QueueHandle_t data_event_queue = NULL;
usb_rate_calc_info_t g_usb_rate_calc_info[2] = {{0,0}, {0,0}};

static uint32_t timestamp_from_sofs = 0;

static void usb_adaptive_clk_manager(void *args) {
    (void) args;

    usb_audio_rate_packet_desc_t pkt_data;
    static uint32_t prev_time = 0;

    while(1) {
        xQueueReceive(data_event_queue, (void *)&pkt_data, portMAX_DELAY);
        g_usb_rate_calc_info[pkt_data.ep_dir] = determine_USB_audio_rate(pkt_data.cur_time, pkt_data.xfer_len, pkt_data.ep_dir);

        prev_time = pkt_data.cur_time;
    }
}

bool tud_xcore_data_cb(uint32_t cur_time, uint32_t ep_num, uint32_t ep_dir, size_t xfer_len)
{
    if ((data_event_queue != NULL) && (ep_num == USB_AUDIO_EP))
    {
        BaseType_t xHigherPriorityTaskWoken;
        usb_audio_rate_packet_desc_t args;
        args.cur_time = timestamp_from_sofs;
        args.ep_num = ep_num;
        args.ep_dir = ep_dir;
        args.xfer_len = xfer_len;
        if( errQUEUE_FULL == xQueueSendFromISR(data_event_queue, (void *)&args, &xHigherPriorityTaskWoken))
        {
            rtos_printf("Audio packet timing event dropped\n");
            xassert(0); /* Missed servicing a data packet */
        }
    }
    return true;
}

bool tud_xcore_sof_cb(uint8_t rhport, uint32_t cur_time)
{
    static uint32_t count = 0;

    count += 1;
    if(count == 8)
    {
        // Log every 8th timestamp to get the timestamp every millisecond. We always assume USB HS operation with bInterval set to 4
        // implying that SOF are received every 125us but data is transferred every 1ms. The number 8 is hardcoded since this is the only
        // supported configuration and bInterval is not configurable for this application.
        timestamp_from_sofs = cur_time;
        count = 0;
    }

    sof_toggle();

    /* False tells TinyUSB to not send the SOF event to the stack */
    return false;
}

void adaptive_rate_adjust_init(void)
{
    xTaskCreate((TaskFunction_t) usb_adaptive_clk_manager,
                "usb_adpt_mgr",
                RTOS_THREAD_STACK_SIZE(usb_adaptive_clk_manager),
                NULL,
                USB_ADAPTIVE_TASK_PRIORITY,
                NULL);

    data_event_queue = xQueueCreate( DATA_EVENT_QUEUE_SIZE, sizeof(usb_audio_rate_packet_desc_t) );
}
