#ifndef SENSOR_COLLECT_H_
#define SENSOR_COLLECT_H_

#include <zephyr/kernel.h>

#define CLSF_OFFSET  (CONFIG_CLSF_WINDOW / 2)

#if CONFIG_CLSF_WINDOW <= CLSF_OFFSET
#error "Window length must be larger than offset"
#endif

#ifdef __cplusplus
extern "C" {
#endif

int64_t TIMER_GetTimeInUS(void);
void MainTask(void);

#ifdef __cplusplus
}
#endif

#endif /* SENSOR_COLLECT_H_ */