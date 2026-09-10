#include <zephyr/ztest.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>

#include "sensor/sensor_raw.h"

ZTEST_SUITE(sensor_init_suite, NULL, NULL, NULL, NULL, NULL);

/* Fehlerpfad: kein Sensor auf native_sim */
ZTEST(sensor_init_suite, test_sensor_init_no_device)
{
    int rc = SENSOR_Init();
    zassert_equal(rc, -ENODEV,
                  "Expected -ENODEV (-19), got %d", rc);
}