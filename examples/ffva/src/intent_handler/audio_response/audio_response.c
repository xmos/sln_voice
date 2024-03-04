// Copyright 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* STD headers */
#include <platform.h>
#include <xs1.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* App headers */
#include "app_conf.h"
#include "platform/driver_instances.h"
#include "intent_handler/intent_handler.h"
#include "audio_response.h"
#include "fs_support.h"
#include "ff.h"
#include "dr_wav_freertos_port.h"

static const char *audio_files_en[] = {
    "50.wav",   /* sleep */
    "1.wav",  /* wakeup */
    "3.wav",  /* tv_on */
    "4.wav",  /* tv_off */
    "5.wav",  /* ch_up */
    "6.wav",  /* ch_down */
    "7.wav",  /* vol_up */
    "8.wav",  /* vol_down */
    "9.wav",  /* lights_on */
    "10.wav",  /* lights_off */
    "11.wav",  /* lights_up*/
    "12.wav",  /* lights_down */
    "13.wav",  /* fan_on */
    "14.wav",  /* fan_off */
    "15.wav",  /* fan_up */
    "16.wav",  /* fan_down */
    "17.wav",  /* temp_up */
    "18.wav",  /* temp_down */
};

#define NUM_FILES (sizeof(audio_files_en) / sizeof(char *))

static int16_t file_audio[appconfAUDIO_PIPELINE_FRAME_ADVANCE * sizeof(int16_t)];
static int32_t i2s_audio[2*(appconfAUDIO_PIPELINE_FRAME_ADVANCE * sizeof(int32_t))];
static drwav *wav_files = NULL;

#pragma stackfunction 3000

int32_t audio_response_init(void) {
    FRESULT result = 0;
    FIL *files = pvPortMalloc(NUM_FILES * sizeof(FIL));
    wav_files = pvPortMalloc(NUM_FILES * sizeof(drwav));

    configASSERT(files);
    configASSERT(wav_files);
    configASSERT(file_audio);
    configASSERT(i2s_audio);

    for (int i=0; i<NUM_FILES; i++) {
        result = f_open(&files[i], audio_files_en[i], FA_READ);
        if (result == FR_OK) {
            drwav_init(
                    &wav_files[i],
                    drwav_read_proc_port,
                    drwav_seek_proc_port,
                    &files[i],
                    &drwav_memory_cbs);
        } else {
            configASSERT(0);
        }
    }
    return 0;
}

#pragma stackfunction 3000
void audio_response_play(int32_t id) {
    drwav tmp;
    size_t framesRead = 0;

    if (wav_files != NULL) {
        if (id < NUM_FILES){  //max id should be (NUM_FILES - 1)
            tmp = wav_files[id];
        } else {
            rtos_printf("No audio response for id %d\n", id);
            return;
        }

        while(1) {
            memset(file_audio, 0x00, sizeof(file_audio));
            framesRead = drwav_read_pcm_frames_s16(&tmp, appconfAUDIO_PIPELINE_FRAME_ADVANCE, file_audio);
            memset(i2s_audio, 0x00, sizeof(i2s_audio));
            for (int i=0; i<framesRead; i++) {
                i2s_audio[(2*i)+0] = (int32_t) file_audio[i] << 16;
                i2s_audio[(2*i)+1] = (int32_t) file_audio[i] << 16;
            }
            
            rtos_i2s_tx(i2s_ctx,
                        (int32_t*) i2s_audio,
                        appconfAUDIO_PIPELINE_FRAME_ADVANCE,
                        portMAX_DELAY);

            if (framesRead != appconfAUDIO_PIPELINE_FRAME_ADVANCE) {
                drwav_seek_to_pcm_frame(&tmp, 0);
                break;
            }
        }
    } else {
        rtos_printf("wav files not initialized\n");
    }
}
