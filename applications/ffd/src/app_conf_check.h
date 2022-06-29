// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef APP_CONF_CHECK_H_
#define APP_CONF_CHECK_H_

#if (appconfUSB_ENABLED == 0) && (appconfINFERENCE_USB_OUTPUT_ENABLED == 1)
#error Inference output over USB is enabled but USB is not enabled
#endif

#endif /* APP_CONF_CHECK_H_ */
