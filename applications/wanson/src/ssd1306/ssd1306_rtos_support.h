// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef SSD1306_RTOS_SUPPORT_H_
#define SSD1306_RTOS_SUPPORT_H_

// display attibs
#define MAX_ROWS 32
#define MAX_COLS 16 // (128/8)
#define TOP_MARGIN 10
#define LEFT_MARGIN 1

// font attribs
#define FONT_SIZE 1

void ssd1306_display_create(unsigned priority);
void ssd1306_display_clear_bitmap(void);
void ssd1306_display_ascii_to_bitmap(char* str_buf);

#endif /* SSD1306_RTOS_SUPPORT_H_ */
