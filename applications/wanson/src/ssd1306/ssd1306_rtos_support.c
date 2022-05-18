// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

/* STD headers */
#include <platform.h>
#include <xs1.h>
#include <xcore/hwtimer.h>
#include <string.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

/* App headers */
#include "app_conf.h"
#include "platform/driver_instances.h"
#include "ssd1306.h"
#include "ssd1306_rtos_support.h"
#include "font8x8_basic.h"

static ssd1306_context ssd1306_ctx_s;
static ssd1306_context* ssd1306_ctx = &ssd1306_ctx_s;

static uint8_t display_buf[512];
static TaskHandle_t display_task_handle = 0;

__attribute__(( fptrgroup("ssd1306_transport_write") ))
size_t ssd1306_I2C_write(void* app_ctx, void* bus, int address, uint8_t *buf, size_t len) {
    size_t num_bytes_sent = 0;

    rtos_i2c_master_write(
            (rtos_i2c_master_t*)bus,
            (uint8_t)address,
            buf,
            len,
            &num_bytes_sent,
            1);

    return num_bytes_sent;
}

static void ssd1306_128x32_clear_bitmap(char* bitmap) {
    // Clear bitmap buffer
    memset(bitmap, 0, sizeof(char)*MAX_ROWS*MAX_COLS);
}

static char* write_line(char* str_buf, char* bitmap, char* brush) {
    char* cur_bmp = NULL;

    while(1) {
        // check for null terminator and edge of canvas
        int next_char_column = ((int)brush - (int)bitmap) % MAX_COLS;
        if (((char)*str_buf == (char)'\0') ||
            (next_char_column + FONT_SIZE >= MAX_COLS)) {
            break;
        }
        // rtos_printf("add %c to bitmap at column %d\n", (char)*str_buf, ((int)brush - (int)bitmap) % MAX_COLS);
        cur_bmp = font8x8_basic[(int)*(str_buf)];

        // draw one character
        for(int x=0; x<sizeof(font8x8_basic[0]); x++) {
            *(brush + (MAX_COLS * x)) = *(cur_bmp + x);
        }
        brush += FONT_SIZE; // move brush over
        str_buf++; // iterate to next character in string
    }
    return str_buf;
}

static void ssd1306_128x32_ascii_to_bitmap(char* str_buf, char* bitmap) {
    char* cur_ptr = str_buf;
    char* brush = NULL;

    // drawing
    int draw_start_pt = (TOP_MARGIN * MAX_COLS) + LEFT_MARGIN; // Give space

    for(int i=0; i<MAX_LINES; i++) {
        brush = bitmap + draw_start_pt + ((sizeof(font8x8_basic[0]) + 1) * i * MAX_COLS);
        cur_ptr = write_line(cur_ptr, bitmap, brush);
        if ((char)*cur_ptr == (char)'\0') {
            break;
        }
    }
}

void display_task(void *args)
{
    ssd1306_transport ssd1306_transport = {i2c_master_ctx, 0x3C, &ssd1306_I2C_write};

    /* Initialize and clear the screen */
    ssd1306_init(
            NULL,
            ssd1306_ctx,
            &ssd1306_transport,
            ssd1306_MDOB128032GV);

    ssd1306_128x32_clear_bitmap((char*)display_buf);

    while(1) {
        ssd1306_write(NULL, ssd1306_ctx, (const uint8_t *)display_buf);
        /* Wait forever until someone tells us we need to update */
        (void) ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
    }
}

void ssd1306_display_create(unsigned priority) {
    xTaskCreate((TaskFunction_t) display_task,
                "display_task",
                RTOS_THREAD_STACK_SIZE(display_task),
                NULL,
                priority,
                &display_task_handle);
}

void ssd1306_display_clear_bitmap(void) {
    ssd1306_128x32_clear_bitmap((char*)display_buf);
    if(display_task_handle != 0) {
        xTaskNotifyGive(display_task_handle);
    }
}

void ssd1306_display_ascii_to_bitmap(char* str_buf) {
    if( str_buf != NULL) {
        ssd1306_128x32_clear_bitmap((char*)display_buf);
        ssd1306_128x32_ascii_to_bitmap(str_buf, (char*)display_buf);
        if(display_task_handle != 0) {
            xTaskNotifyGive(display_task_handle);
        }
    }
}
