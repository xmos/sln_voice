// Copyright 2021-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
// XMOS Public License: Version 1

#ifndef HOSTACTIVE_H
#define HOSTACTIVE_H

/**
 * @brief Initialise the host active LED GPO port
 *
 */
void UserHostActive_LED_Init(void);

/**
 * @brief Configure the host active LED
 *
 * @param active LED active status
 */
void UserHostActive(int active);

#endif
