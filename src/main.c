#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "sensor/sensor_collect.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

int main(void)
{
    LOG_INF("System ready");
    k_msleep(500);
    MainTask();
    return 0;
}