/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <our_drivers/led_sensor.h>

#define DT_DRV_COMPAT our_led_sensor

LOG_MODULE_REGISTER(led_sensor, CONFIG_SENSOR_LOG_LEVEL);

struct led_sensor_config {
	struct gpio_dt_spec led; /* read-only, from DTS */
};

struct led_sensor_data {
	int32_t led_on; /* LED state captured by the last sample fetch */
	bool keep_on;   /* when set, channel_get leaves the LED on */
};

static int led_sensor_sample_fetch(const struct device *dev,
				   enum sensor_channel chan)
{
	const struct led_sensor_config *cfg = dev->config;
	struct led_sensor_data *data = dev->data;
	int ret;

	if (chan != SENSOR_CHAN_ALL && chan != SENSOR_CHAN_PROX) {
		return -ENOTSUP;
	}

	ret = gpio_pin_set_dt(&cfg->led, 1);
	if (ret < 0) {
		return ret;
	}

	data->led_on = 1;
	LOG_DBG("LED on");

	return 0;
}

static int led_sensor_channel_get(const struct device *dev,
				  enum sensor_channel chan,
				  struct sensor_value *val)
{
	const struct led_sensor_config *cfg = dev->config;
	struct led_sensor_data *data = dev->data;
	int ret;

	if (chan != SENSOR_CHAN_PROX) {
		return -ENOTSUP;
	}

	/* Report the state seen by the last fetch, then switch the LED off. */
	val->val1 = data->led_on;
	val->val2 = 0;

	if (data->keep_on) {
		return 0;
	}

	ret = gpio_pin_set_dt(&cfg->led, 0);
	if (ret < 0) {
		return ret;
	}

	data->led_on = 0;
	LOG_DBG("LED off");

	return 0;
}

int led_sensor_set_keep_on(const struct device *dev, bool keep_on)
{
	struct led_sensor_data *data;

	if (dev == NULL) {
		return -EINVAL;
	}

	data = dev->data;
	data->keep_on = keep_on;
	LOG_DBG("keep_on = %d", keep_on);

	return 0;
}

static DEVICE_API(sensor, led_sensor_api) = {
	.sample_fetch = led_sensor_sample_fetch,
	.channel_get = led_sensor_channel_get,
};

static int led_sensor_init(const struct device *dev)
{
	const struct led_sensor_config *cfg = dev->config;

	if (!gpio_is_ready_dt(&cfg->led)) {
		LOG_ERR("LED GPIO controller not ready");
		return -ENODEV;
	}

	return gpio_pin_configure_dt(&cfg->led, GPIO_OUTPUT_INACTIVE);
}

#define LED_SENSOR_DEFINE(inst)                                                \
	static struct led_sensor_data data_##inst;                             \
	static const struct led_sensor_config cfg_##inst = {                   \
		.led = GPIO_DT_SPEC_INST_GET(inst, gpios),                     \
	};                                                                     \
	SENSOR_DEVICE_DT_INST_DEFINE(inst, led_sensor_init, NULL,              \
				     &data_##inst, &cfg_##inst, POST_KERNEL,   \
				     CONFIG_SENSOR_INIT_PRIORITY,              \
				     &led_sensor_api)

DT_INST_FOREACH_STATUS_OKAY(LED_SENSOR_DEFINE)
