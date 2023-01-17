// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef APP_CONF_CHECK_H_
#define APP_CONF_CHECK_H_

#if XCOREAI_EXPLORER
#if appconfSSD1306_DISPLAY_ENABLED != 0
#error SSD1306 support is not available on XCOREAI_EXPLORER board
#endif
#endif

#endif /* APP_CONF_CHECK_H_ */
