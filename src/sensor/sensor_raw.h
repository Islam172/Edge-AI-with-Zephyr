#ifndef SENSOR_RAW_H_
#define SENSOR_RAW_H_

#include <zephyr/kernel.h>

#ifdef __cplusplus
extern "C" {
#endif

int SENSOR_Init(void);
int SENSOR_Run(int16_t *rawSensorData);

#ifdef __cplusplus
}
#endif

#endif /* SENSOR_RAW_H_ */