// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef POWER_STATE_H_
#define POWER_STATE_H_

typedef struct {
    float vnr_pred;
    float ema_energy;
} power_data_t;

typedef enum power_state {
    POWER_STATE_LOW,
    POWER_STATE_FULL
} power_state_t;

void power_state_init();
void power_state_set(power_state_t state);
power_state_t power_state_data_add(power_data_t *data);

#endif /* POWER_STATE_H_ */
