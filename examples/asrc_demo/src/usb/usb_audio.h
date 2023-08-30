// Copyright (c) 2021-2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef USB_AUDIO_H_
#define USB_AUDIO_H_

/*
 * frame_buffers format assumes:
 *   processed_audio_frame
 *   reference_audio_frame
 *   raw_mic_audio_frame
 */
void usb_audio_send(int32_t *frame_buffer_ptr,
                    size_t frame_count,
                    size_t num_chans);

void usb_audio_init(rtos_intertile_t *intertile_ctx, unsigned priority);


#endif /* USB_AUDIO_H_ */
