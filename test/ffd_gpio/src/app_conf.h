// Copyright 2021-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef APP_CONF_H_
#define APP_CONF_H_

/* Intertile Communication Configuration */
#define INTERTILE_RPC_PORT 10
#define INTERTILE_RPC_HOST_TASK_PRIORITY (configMAX_PRIORITIES-1)

#define INTERTILE_TEST_SYNC_PORT 11
#define INTERTILE_TEST_SYNC_TASK_PRIORITY (configMAX_PRIORITIES-1)

#define I2C_SLAVE_ISR_CORE   4
#define I2C_SLAVE_CORE_MASK  (1 << 3)
#define I2C_SLAVE_ADDR       0x7A

#define appconfINTENT_I2C_OUTPUT_DEVICE_ADDR  I2C_SLAVE_ADDR

#define appconfAUDIO_PLAYBACK_ENABLED           0
#define appconfINTENT_I2C_OUTPUT_ENABLED     0
#define appconfINTENT_UART_OUTPUT_ENABLED    0

#define ASR_TILE_NO                             0
#define UART_TILE_NO                            0
#define appconfINTENT_WAKEUP_EDGE_TYPE          0

#define UART_RX_CORE_MASK       (1 << 2)
#define UART_RX_ISR_CORE        2
#define UART_BAUD_RATE          921600

#define appconfINTENT_TRANSPORT_DELAY_MS     1

#if ON_TILE(1)
#define GPIO_HOST_TO_BOARD_HOST_STATUS_PORT XS1_PORT_1P
#define GPIO_BOARD_TO_HOST_WAKEUP_PORT      XS1_PORT_1M
#endif

#define GPIO_OUT_HOST_WAKEUP_PORT   XS1_PORT_1M  /* X0D36 */
#define GPIO_IN_HOST_STATUS_PORT    XS1_PORT_1P  /* X0D39 */

/* Task Priorities */
#define appconfSTARTUP_TASK_PRIORITY            ( configMAX_PRIORITIES - 1 )

#endif /* APP_CONF_H_ */
