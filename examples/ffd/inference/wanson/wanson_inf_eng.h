// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef WANSON_INF_ENG_H_
#define WANSON_INF_ENG_H_

#define IS_WAKEWORD(id)   (id < 200)
#define IS_COMMAND(id)    (id >= 200)

void wanson_engine_task(void *args);

void wanson_engine_task_create(unsigned priority);
void wanson_engine_samples_send_local(
        size_t frame_count,
        int32_t *processed_audio_frame);

void wanson_engine_intertile_task_create(uint32_t priority);
void wanson_engine_samples_send_remote(
        rtos_intertile_t *intertile_ctx,
        size_t frame_count,
        int32_t *processed_audio_frame);

void wanson_engine_proc_keyword_result(const char **text, int id);

#endif /* WANSON_INF_ENG_H_ */
