#define DEBUG_UNIT RATE_SERVER
#define DEBUG_PRINT_ENABLE_RATE_SERVER 1

/* STD headers */
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "rtos_printf.h"
#include <xcore/hwtimer.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "stream_buffer.h"

/* App headers */
#include "app_conf.h"
#include "platform/driver_instances.h"
#include "src.h"
#include "asrc_utils.h"
#include "i2s_audio.h"

extern uint32_t dsp_math_divide_unsigned_64(uint64_t dividend, uint32_t divisor, uint32_t q_format );


void rate_server(void *args)
{
    uint32_t prev_ts = get_reference_time();
    uint32_t prev_num_i2s_samples_recvd = i2s_ctx->recv_buffer.total_written;
    for(;;)
    {
        vTaskDelay(pdMS_TO_TICKS(20));
        uint32_t current_ts = get_reference_time();
        uint32_t current_num_i2s_samples = i2s_ctx->recv_buffer.total_written;

        uint32_t samples = (current_num_i2s_samples - prev_num_i2s_samples_recvd) >> 1; // 2 channels per sample, 4 bytes per channel
        uint32_t timespan = current_ts - prev_ts;
        uint64_t total_data = (uint64_t)(samples) * 12500;
        uint32_t data_per_sample = dsp_math_divide_unsigned_64(total_data, (timespan / 8), 19);

        //printuintln(data_per_sample);
        //printuintln(samples);
        //printuintln(current_ts - prev_ts);

        prev_num_i2s_samples_recvd = current_num_i2s_samples;
        prev_ts = current_ts;

        uint8_t tmp = 0;
        rtos_intertile_tx(
            intertile_ctx,
            appconfUSB_RATE_NOTIFY_PORT,
            &tmp,
            sizeof(tmp));

        size_t bytes_received;
        uint32_t usb_rate_ratio;
        bytes_received = rtos_intertile_rx_len(
                    intertile_ctx,
                    appconfUSB_RATE_NOTIFY_PORT,
                    portMAX_DELAY);
        xassert(bytes_received == sizeof(usb_rate_ratio));

        rtos_intertile_rx_data(
                        intertile_ctx,
                        &usb_rate_ratio,
                        bytes_received);

        // Get the I2S rate

    }
}
