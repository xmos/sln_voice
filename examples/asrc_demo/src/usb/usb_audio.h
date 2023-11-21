// Copyright 2021-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef USB_AUDIO_H_
#define USB_AUDIO_H_

// These are used by the dbtomult and fixed point volume scaling calcs
#define USB_AUDIO_VOL_MUL_FRAC_BITS     29
#define USB_AUDIO_VOLUME_FRAC_BITS      8

// Volume feature unit range in decibels
// These are stored in 8.8 values in decibels. Max volume is 0dB which is 1.0 gain because we only attenuate.
#define USB_AUDIO_MIN_VOLUME_DB     ((int16_t)-60  << USB_AUDIO_VOLUME_FRAC_BITS)
#define USB_AUDIO_MAX_VOLUME_DB     ((int16_t)0  << USB_AUDIO_VOLUME_FRAC_BITS)
#define USB_AUDIO_VOLUME_STEP_DB    ((int16_t)1  << USB_AUDIO_VOLUME_FRAC_BITS)

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
