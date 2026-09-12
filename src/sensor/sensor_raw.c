#include "sensor_raw.h"

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(sensor_raw, LOG_LEVEL_INF);

#define ACCEL_NODE DT_ALIAS(accel0)

#if DT_NODE_EXISTS(ACCEL_NODE)
static const struct device *accel_dev;
#endif

int SENSOR_Init(void)
{
#if !DT_NODE_EXISTS(ACCEL_NODE)
    LOG_ERR("accel0 alias not defined in DeviceTree");
    return -ENODEV;
#else
    accel_dev = DEVICE_DT_GET(ACCEL_NODE);
    if (!device_is_ready(accel_dev)) {
        LOG_ERR("Sensor device not ready");
        return -ENODEV;
    }
    LOG_INF("Sensor initialized successfully");
    return 0;
#endif
}

int SENSOR_Run(int16_t *rawSensorData)
{
#if !DT_NODE_EXISTS(ACCEL_NODE)
    return -ENODEV;
#else
    struct sensor_value accel[3];
    int rc;

    rc = sensor_sample_fetch(accel_dev);
    if (rc != 0) {
        LOG_ERR("sensor_sample_fetch failed: %d", rc);
        return rc;
    }

    rc = sensor_channel_get(accel_dev,
                            SENSOR_CHAN_ACCEL_XYZ,
                            accel);
    if (rc != 0) {
        LOG_ERR("sensor_channel_get failed: %d", rc);
        return rc;
    }

    rawSensorData[0] = (int16_t)sensor_value_to_double(&accel[0]);
    rawSensorData[1] = (int16_t)sensor_value_to_double(&accel[1]);
    rawSensorData[2] = (int16_t)sensor_value_to_double(&accel[2]);

    return 0;
#endif
}