/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/shell/shell.h>

#include <errno.h>
#include <stdlib.h>

#include <our_drivers/led_sensor.h>

static const struct device *const led_sensor = DEVICE_DT_GET_ANY(our_led_sensor);

static int check_ready(const struct shell *sh)
{
	if (led_sensor == NULL || !device_is_ready(led_sensor)) {
		shell_error(sh, "LED sensor is not ready");
		return -ENODEV;
	}

	return 0;
}

static int cmd_sensor_fetch(const struct shell *sh, size_t argc, char **argv)
{
	int ret;

	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	ret = check_ready(sh);
	if (ret < 0) {
		return ret;
	}

	ret = sensor_sample_fetch(led_sensor);
	if (ret < 0) {
		shell_error(sh, "sensor_sample_fetch failed (%d)", ret);
		return ret;
	}

	shell_print(sh, "Sample fetched, LED on");

	return 0;
}

static int cmd_sensor_read(const struct shell *sh, size_t argc, char **argv)
{
	struct sensor_value val;
	int ret;

	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	ret = check_ready(sh);
	if (ret < 0) {
		return ret;
	}

	ret = sensor_channel_get(led_sensor, SENSOR_CHAN_PROX, &val);
	if (ret < 0) {
		shell_error(sh, "sensor_channel_get failed (%d)", ret);
		return ret;
	}

	shell_print(sh, "LED was: %s (val1=%d, val2=%d)", val.val1 ? "ON" : "OFF", val.val1,
		    val.val2);

	return 0;
}

static int cmd_sensor_info(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	if (led_sensor == NULL) {
		shell_error(sh, "No LED sensor device found");
		return -ENODEV;
	}

	shell_print(sh, "Device: %s", led_sensor->name);
	shell_print(sh, "Ready:  %s", device_is_ready(led_sensor) ? "yes" : "no");

	return 0;
}

static int cmd_sensor_set(const struct shell *sh, size_t argc, char **argv)
{
	char *end;
	long value;
	int ret;

	ARG_UNUSED(argc);

	ret = check_ready(sh);
	if (ret < 0) {
		return ret;
	}

	errno = 0;
	value = strtol(argv[1], &end, 0);
	if (errno != 0 || end == argv[1] || *end != '\0') {
		shell_error(sh, "Invalid value '%s': expected a number (0 or 1)", argv[1]);
		return -EINVAL;
	}

	if (value < 0 || value > 1) {
		shell_error(sh, "Value %ld out of range: expected 0 or 1", value);
		return -ERANGE;
	}

	ret = led_sensor_set_keep_on(led_sensor, value != 0);
	if (ret < 0) {
		shell_error(sh, "led_sensor_set_keep_on failed (%d)", ret);
		return ret;
	}

	shell_print(sh, "keep_on = %ld (read %s the LED on)", value,
		    value ? "leaves" : "turns off");

	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_sensor,
	SHELL_CMD(fetch, NULL, "Fetch a sample (turns the LED on)", cmd_sensor_fetch),
	SHELL_CMD(read, NULL, "Read the channel (turns the LED off)", cmd_sensor_read),
	SHELL_CMD(info, NULL, "Show device name and ready state", cmd_sensor_info),
	SHELL_CMD_ARG(set, NULL, "Set keep_on: 1 keeps the LED on after read, 0 turns it off\n"
				 "Usage: sensor set <0|1>", cmd_sensor_set, 2, 0),
	SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(sensor, &sub_sensor, "LED sensor commands", NULL);
