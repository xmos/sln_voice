
#define DEBUG_UNIT I2S_USB_INTERTILE
#define DEBUG_PRINT_ENABLE_I2S_USB_INTERTILE 1

#include <platform.h>
#include <xs1.h>
#include <xcore/channel.h>

#include <rtos_printf.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"
#include "queue.h"

/* Library headers */
#include "rtos_printf.h"
#include "src.h"

/* App headers */
#include "app_conf.h"
#include "platform/platform_init.h"
#include "platform/driver_instances.h"
#include "usb_support.h"
#include "usb_audio.h"
#include "fs_support.h"

#include "gpio_test/gpio_test.h"
#include "i2s_audio.h"
#include "rate_server.h"
#include "asrc_utils.h"
#include "tusb_config.h"

int32_t g_avg_i2s_send_buffer_level = 0;
int32_t g_prev_avg_i2s_send_buffer_level = 0;

extern bool g_spkr_itf_close_to_open;

static void calc_avg_i2s_buffer_level(int current_level, bool reset)
{
    static int64_t error_accum = 0;
    static int32_t count = 0;

    if(reset == true)
    {
        error_accum = 0;
        count = 0;
        g_avg_i2s_send_buffer_level = 0;
        g_prev_avg_i2s_send_buffer_level = 0;
        rtos_printf("Reset avg I2S send buffer level\n");
    }

    error_accum += current_level;
    count += 1;

    if(count == 0x10000)
    {
        g_prev_avg_i2s_send_buffer_level = g_avg_i2s_send_buffer_level;
        g_avg_i2s_send_buffer_level = error_accum >> 16;
        count = 0;
        error_accum = 0;
    }
}

static unsigned usb_audio_recv(rtos_intertile_t *intertile_ctx,
                        int32_t **frame_buffers)
{
    static int32_t frame_samples_interleaved[(USB_TO_I2S_ASRC_BLOCK_LENGTH * 4) + 10][CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX]; //+1 should be okay but +10 just in case

    size_t bytes_received;

    bytes_received = rtos_intertile_rx_len(
        intertile_ctx,
        appconfUSB_AUDIO_PORT,
        portMAX_DELAY);

    if (bytes_received > 0)
    {
        xassert(bytes_received <= sizeof(frame_samples_interleaved));

        rtos_intertile_rx_data(
            intertile_ctx,
            frame_samples_interleaved,
            bytes_received);

        *frame_buffers = &frame_samples_interleaved[0][0];
        return bytes_received >> 3; // Return number of 32bit samples per channel. 4bytes per samples and 2 channels
    }
    else
    {
        return 0;
    }
}


// USB audio recv -> ASRC -> |to other tile| -> usb_to_i2s_intertile -> I2S send
void usb_to_i2s_intertile(void *args) {
    (void) args;
    int32_t *usb_to_i2s_samps;
    for(;;)
    {
        // Receive USB recv + ASRC data frame from the other tile
        unsigned num_samps = usb_audio_recv(intertile_usb_audio_ctx,
                                            &usb_to_i2s_samps
                                        );

        int i2s_send_buffer_unread;
        int i2s_buffer_level_from_half;
        static uint32_t prev_i2s_sampling_rate = 0;

        //printf("-- %d --\n",i2s_send_buffer_unread/2);
        if(num_samps)
        {
            i2s_send_buffer_unread = i2s_ctx->send_buffer.total_written - i2s_ctx->send_buffer.total_read;
            if(g_spkr_itf_close_to_open == true)
            {
                if(i2s_send_buffer_unread > 0)
                {
                    i2s_ctx->okay_to_send = true;
                    rtos_printf("USB spkr interface opened. i2s_send_buffer_unread = %d. fill level = %d\n", i2s_send_buffer_unread, (signed)((signed)i2s_send_buffer_unread - (i2s_ctx->send_buffer.buf_size / 2)));
                    // Wait for i2s send buffer to drain fully before refilling it, so there's no chance we start at a fill level greater than 0
                    continue;
                }
                rtos_printf("USB spkr interface opened. i2s_send_buffer_unread = %d. fill level = %d\n", i2s_send_buffer_unread, (signed)((signed)i2s_send_buffer_unread - (i2s_ctx->send_buffer.buf_size / 2)));
                g_spkr_itf_close_to_open = false;
                i2s_ctx->okay_to_send = false; // We wait for buffer to be half full before resuming send on I2S
            }

            // If there's a change in sampling rate detected
            if(prev_i2s_sampling_rate != i2s_ctx->i2s_nominal_sampling_rate)
            {
                if(i2s_send_buffer_unread > 0)
                {
                    i2s_ctx->okay_to_send = true;
                    rtos_printf("I2S sampling rate change detected. prev = %u, current = %u. i2s_send_buffer_unread = %d. fill level = %d\n", prev_i2s_sampling_rate, i2s_ctx->i2s_nominal_sampling_rate, i2s_send_buffer_unread, (signed)((signed)i2s_send_buffer_unread - (i2s_ctx->send_buffer.buf_size / 2)));
                    // Wait for i2s send buffer to drain fully before refilling it, so there's no chance we start at a fill level greater than 0
                    continue;
                }
                rtos_printf("I2S sampling rate change detected. prev = %u, current = %u. i2s_send_buffer_unread = %d, fill level = %d\n", prev_i2s_sampling_rate, i2s_ctx->i2s_nominal_sampling_rate, i2s_send_buffer_unread, (signed)((signed)i2s_send_buffer_unread - (i2s_ctx->send_buffer.buf_size / 2)));
                prev_i2s_sampling_rate = i2s_ctx->i2s_nominal_sampling_rate;
                i2s_ctx->okay_to_send = false;
            }

            rtos_i2s_tx(i2s_ctx,
                (int32_t*) usb_to_i2s_samps,
                num_samps,
                portMAX_DELAY);

            i2s_send_buffer_unread = i2s_ctx->send_buffer.total_written - i2s_ctx->send_buffer.total_read;
            i2s_buffer_level_from_half = (signed)((signed)i2s_send_buffer_unread - (i2s_ctx->send_buffer.buf_size / 2));    //Level w.r.t. half full.
            calc_avg_i2s_buffer_level(i2s_buffer_level_from_half / 2, !i2s_ctx->okay_to_send); // Per channel

            //printintln((i2s_buffer_level_from_half / 2));

            if((i2s_ctx->okay_to_send == false) && (i2s_buffer_level_from_half >= 0))
            {
                rtos_printf("Start sending over I2S. I2S send buffer fill level = %d\n", i2s_buffer_level_from_half);
                i2s_ctx->okay_to_send = true;
            }
        }
    }
}
