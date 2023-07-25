#pragma once

#include <xcore/parallel.h>
#include <xcore/chanend.h>

#ifdef __cplusplus
extern "C" {
#endif
void ma_init();
void ma_task(chanend_t c_mic_to_audio);
#ifdef __cplusplus
}
#endif
