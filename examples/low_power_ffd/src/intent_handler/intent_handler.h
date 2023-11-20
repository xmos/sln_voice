// Copyright 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef INTENT_HANDLER_H_
#define INTENT_HANDLER_H_

#if XK_VOICE_L71
#define GPIO_OUT_HOST_WAKEUP_PORT   XS1_PORT_1D  /* PORT_SPI_MOSI */
#define GPIO_IN_HOST_STATUS_PORT    XS1_PORT_1P  /* PORT_SPI_MISO */

#elif XCOREAI_EXPLORER
#define GPIO_OUT_HOST_WAKEUP_PORT   XS1_PORT_1M  /* X0D36 */
#define GPIO_IN_HOST_STATUS_PORT    XS1_PORT_1P  /* X0D39 */

#else
#define GPIO_OUT_HOST_WAKEUP_PORT   0
#define GPIO_IN_HOST_STATUS_PORT    0
#endif

int32_t intent_handler_create(uint32_t priority, void *args);

#endif /* INTENT_HANDLER_H_ */
