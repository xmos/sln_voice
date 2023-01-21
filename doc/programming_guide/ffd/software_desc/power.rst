.. _sln_voice_ffd_power:

#########
power
#########


Overview
========

This folder contains modules for lower power control and state reporting in the
FFD application.


Configuration Notes
===================

The application monitors the VNR and produces low power events based on:

- `appconfPOWER_VNR_THRESHOLD`
- `appconfPOWER_LOW_ENERGY_THRESHOLD`
- `appconfPOWER_HIGH_ENERGY_THRESHOLD`
- `appconfPOWER_FULL_HOLD_DURATION`

The first three configuration options above determine when to produce a
`POWER_STATE_FULL` event. The last configuration option above determines when
a timer period, on expiration, this timer will produce a `POWER_STATE_LOW`
event. Additionally, when `POWER_STATE_FULL` events occur, this timer is reset
thus keeping the device is `POWER_STATE_FULL` or at least another
`appconfPOWER_FULL_HOLD_DURATION` milliseconds.

`appconfPOWER_FULL_HOLD_DURATION` should be configured to take into account
`appconfINFERENCE_RESET_DELAY_MS` and any time required to produce
(audible/visual) feedback; in this application, the SLEEP_WAV tone should be
considered.

`appconfLOW_POWER_ENABLED` enables/disables use of this low power functionality.

When `appconfLOW_POWER_SWITCH_CLK_DIV_ENABLE` is enabled,
`appconfLOW_POWER_SWITCH_CLK_DIV` should be set appropriately. Clock divider
values that result in frequencies greater than or equal to 20MHz have been
observed to work.

Values for `appconfLOW_POWER_CONTROL_TILE_CLK_DIV` that result in frequencies
greater than or equal to 200MHz have been observed to work.