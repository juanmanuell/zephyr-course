# SPDX-License-Identifier: Apache-2.0

if(CONFIG_OUR_LED_SENSOR)
  zephyr_library()
  zephyr_library_sources(led_sensor.c)
endif()
