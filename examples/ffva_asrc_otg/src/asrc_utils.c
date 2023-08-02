/* STD headers */
#include <string.h>
#include <stdint.h>
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

//Helper function for converting sample to fs index value
fs_code_t samp_rate_to_code(unsigned samp_rate){
    unsigned samp_code = 0xdead;
    switch (samp_rate){
    case 44100:
        samp_code = FS_CODE_44;
        break;
    case 48000:
        samp_code = FS_CODE_48;
        break;
    case 88200:
        samp_code = FS_CODE_88;
        break;
    case 96000:
        samp_code = FS_CODE_96;
        break;
    case 176400:
        samp_code = FS_CODE_176;
        break;
    case 192000:
        samp_code = FS_CODE_192;
        break;
    }
    return samp_code;
}
