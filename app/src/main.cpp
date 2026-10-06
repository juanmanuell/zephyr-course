#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

/* The devicetree node identifier for the "app-led" alias. */
#define LED_NODE DT_ALIAS(app_led)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

int main(void)
{
    bool led_state = true;
    struct sensor_value val;
    const struct device *led_sensor = DEVICE_DT_GET_ANY(our_led_sensor);

    if (!gpio_is_ready_dt(&led)) return 0;

    if (gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE) < 0) return 0;

    if (!device_is_ready(led_sensor)) {
        LOG_ERR("LED sensor not ready");
        return 0;
    }

    while (1) {
        if (gpio_pin_toggle_dt(&led) < 0) return 0;

        led_state = !led_state;
        LOG_INF("LED state: %s", led_state ? "ON" : "OFF");

        /* The fetch turns the sensor LED on, the channel read turns it off. */
        if (sensor_sample_fetch(led_sensor) < 0) {
            LOG_ERR("sensor_sample_fetch failed");
        }
        k_msleep(CONFIG_APP_HEARTBEAT_PERIOD_MS / 2);

        if (sensor_channel_get(led_sensor, SENSOR_CHAN_PROX, &val) < 0) {
            LOG_ERR("sensor_channel_get failed");
        } else {
            LOG_INF("Sensor LED was: %s", val.val1 ? "ON" : "OFF");
        }
        k_msleep(CONFIG_APP_HEARTBEAT_PERIOD_MS / 2);
    }
    return 0;
}
