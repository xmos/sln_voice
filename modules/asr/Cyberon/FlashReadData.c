// Copyright 2021-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <stdint.h>
#include <xs1.h>
#include <xcore/assert.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"

/* Library headers */
#include "asr.h"
#include "rtos_qspi_flash.h"

/* App headers */
#include "platform/driver_instances.h"
#include "FlashReadData.h"

#define DBG_TRACE                 // Output log by DbgTrace(), if not defined only output by rtos_printf.
#include "DbgTrace.h"

#if 1

extern devmem_manager_t *devmem_ctx;

size_t FlashReadData(void * dst, const void * src, size_t size)
{
    devmem_read_ext(devmem_ctx, dst, src, size); 
	return size;
}

#else

// Slower than the upper version, about 12 MCPS.
size_t FlashReadData(void *dest, const void *src, size_t size)
{
	int n = -1;
	int nTryCount = 0;
	uint32_t uSrc = (uint32_t)src - XS1_SWMEM_BASE;
	
	while (n != 0 && nTryCount++ < 1000)
	{
		n = rtos_qspi_flash_fast_read_mode_ll(qspi_flash_ctx, (uint8_t *)dest, uSrc, size, qspi_fast_flash_read_transfer_raw); //Model need nibble swap.
		if (n == 0)
			return nTryCount;
		else
			vTaskDelay(1);
	}
	
	DBG_TRACE("Error: try rtos_qspi_flash_fast_read_mode_ll() over 1000!\r\n");
	return 0;
}

#endif