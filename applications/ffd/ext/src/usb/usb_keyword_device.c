// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"

#include "usb_descriptors.h"
#include "tusb.h"
#include "app_conf.h"
#include "usb_keyword_device.h"

#if appconfINFERENCE_USB_OUTPUT_ENABLED
static uint32_t last_intent = 0;

SemaphoreHandle_t xSemaphore;

// Invoked when received new data
TU_ATTR_WEAK void tud_vendor_rx_cb(uint8_t itf) {
    (void) itf;

    /* TODO, should check that this was the desired opcode */
    tud_vendor_read_flush();

    if(xSemaphore != NULL) {
        xSemaphoreTake(xSemaphore, portMAX_DELAY);
        /* If the user has asked for latest keyword status */
        tud_vendor_write(&last_intent, sizeof(last_intent));
        last_intent = 0;
        xSemaphoreGive(xSemaphore);
    }

    tud_vendor_flush();
}

// Invoked when last rx transfer finished
TU_ATTR_WEAK void tud_vendor_tx_cb(uint8_t itf, uint32_t sent_bytes) {
    (void) itf;
    (void) sent_bytes;
}

void usb_keyword_update(int id) {
    if(xSemaphore == NULL) {
        xSemaphore = xSemaphoreCreateBinary();
        xSemaphoreGive(xSemaphore);
    }

    xSemaphoreTake(xSemaphore, portMAX_DELAY);
    last_intent = (uint32_t)id;
    xSemaphoreGive(xSemaphore);
}
#endif
