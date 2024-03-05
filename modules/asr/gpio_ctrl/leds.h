// Copyright 2022-2024 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef LEDS_H_
#define LEDS_H_

/**
 * @brief Create the LED task.
 *
 * @param priority The priority of the task.
 * @param args The arguments to send to the task.
 */
void led_task_create(unsigned priority, void *args);

/**
 * @brief To be called only by the power control task. Indicates that the
 * device is entering low power mode.
 */
void led_indicate_asleep(void);

/**
 * @brief To be called only by the power control task. Indicates that the device
 * has exited low power mode.
 */
void led_indicate_awake(void);

/**
 * @brief To be called when waiting for the wake word or after a timeout period
 * waiting for a spoken command.
 */
void led_indicate_waiting(void);

/**
 * @brief To be called when listening for a command.
 */
void led_indicate_listening(void);

/**
 * @brief To be called when evaluation period ends.
 */
void led_indicate_end_of_eval(void);

#endif /* LEDS_H_ */