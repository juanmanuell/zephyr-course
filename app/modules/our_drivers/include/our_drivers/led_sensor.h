/*
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef OUR_DRIVERS_LED_SENSOR_H_
#define OUR_DRIVERS_LED_SENSOR_H_

#include <stdbool.h>

#include <zephyr/device.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Choose whether sensor_channel_get() switches the LED off.
 *
 * Custom extension of the sensor API. It changes the `keep_on` field in the
 * driver's runtime data. When set, sensor_channel_get() leaves the LED on
 * instead of turning it off.
 *
 * @param dev     LED sensor device
 * @param keep_on true to keep the LED on after a channel read
 *
 * @retval 0 on success
 * @retval -EINVAL if @p dev is NULL
 */
int led_sensor_set_keep_on(const struct device *dev, bool keep_on);

#ifdef __cplusplus
}
#endif

#endif /* OUR_DRIVERS_LED_SENSOR_H_ */
