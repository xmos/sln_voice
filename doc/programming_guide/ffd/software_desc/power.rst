.. _sln_voice_ffd_power:

#########
src/power
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

The first three configuration options above determine when to begin
transitioning, or continue to hold, the device in `POWER_STATE_FULL`. The last
configuration option above determines the minimum period of time to device is
allowed to wait for a wake word before requesting to transition into
`POWER_STATE_LOW`. Each time `POWER_STATE_FULL` is set by the audio pipeline
tile, the timer that is configured for a period of `appconfPOWER_FULL_HOLD_DURATION`
milliseconds is reset, preventing any requests to `POWER_STATE_LOW` to be
aborted.

`appconfLOW_POWER_ENABLED` enables/disables use of this low power functionality.

When `appconfLOW_POWER_SWITCH_CLK_DIV_ENABLE` is enabled,
`appconfLOW_POWER_SWITCH_CLK_DIV` should be set appropriately. Clock divider
values that result in frequencies greater than or equal to 20MHz have been
observed to work.

Values for `appconfLOW_POWER_CONTROL_TILE_CLK_DIV` that result in frequencies
greater than or equal to 300MHz have been observed to work.