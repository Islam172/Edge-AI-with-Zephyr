#include "sensor_collect.h"
#include "sensor_raw.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <string.h>

LOG_MODULE_REGISTER(sensor_collect, LOG_LEVEL_INF);

#ifdef CONFIG_SENSOR_COLLECT_RUN_INFERENCE
  #ifdef CONFIG_SENSOR_INFENG_EMLEARN
    #include "inference_engine/emlearn/emlearn_model.h"
    static int (*SNS_MODEL_Init)(void) = &EMLEARN_MODEL_Init;
    static int (*SNS_MODEL_RunInference)(void *inputData, size_t size,
        int8_t *predClass, int32_t *tinf_us, uint8_t verbose) =
        &EMLEARN_MODEL_RunInference;
  #endif
  static float g_clsfInputData[CONFIG_CLSF_CHANNELS * CONFIG_CLSF_WINDOW] __aligned(32);
#endif

/* Sensordaten-Struktur */
struct sensor_data {
    uint64_t sampleNum;
    uint64_t ts_us;
    int16_t  rawDataSensor[3];
};

/* Queue */
K_MSGQ_DEFINE(g_sensorCollectQueue,
              sizeof(struct sensor_data),
              CONFIG_SENSOR_COLLECT_QUEUE_ITEMS,
              4);

/* Timer */
static struct k_timer g_sensorCollectTimer;

/* Zeitbasis */
int64_t TIMER_GetTimeInUS(void)
{
    return k_uptime_get() * 1000LL;
}

/* ---- CSV-Logger Thread ---- */
#ifdef CONFIG_SENSOR_COLLECT_LOG_EXT
static void sensor_log_thread(void *p1, void *p2, void *p3)
{
    struct sensor_data data;

    while (1) {
        if (k_msgq_get(&g_sensorCollectQueue, &data, K_FOREVER) == 0) {
            printk("%d,%d,%d\r\n",
                   data.rawDataSensor[0],
                   data.rawDataSensor[1],
                   data.rawDataSensor[2]);
        }
    }
}

K_THREAD_DEFINE(log_thread, 2048,
                sensor_log_thread, NULL, NULL, NULL,
                K_PRIO_PREEMPT(2), 0, 0);
#endif /* CONFIG_SENSOR_COLLECT_LOG_EXT */

/* ---- Inference Thread ---- */
#ifdef CONFIG_SENSOR_COLLECT_RUN_INFERENCE
static void sensor_inf_thread(void *p1, void *p2, void *p3)
{
    int rc = SNS_MODEL_Init();
    if (rc != 0) {
        LOG_ERR("Model init failed: %d", rc);
        return;
    }

    struct sensor_data data;
    static uint16_t clsfSampIdx = 0;

    while (1) {
        if (k_msgq_get(&g_sensorCollectQueue, &data, K_FOREVER) == 0) {

            for (int i = 0; i < CONFIG_CLSF_CHANNELS; i++) {
                float sens_val = (float)data.rawDataSensor[i];

#ifdef CONFIG_SENSOR_DATA_FORMAT_INTERLEAVED
                g_clsfInputData[CONFIG_CLSF_CHANNELS * clsfSampIdx + i] = sens_val;
#else
                g_clsfInputData[i * CONFIG_CLSF_WINDOW + clsfSampIdx] = sens_val;
#endif
            }

            if (++clsfSampIdx >= CONFIG_CLSF_WINDOW) {
                int32_t tinf_us = 0;
                int8_t  predClass = -1;

                SNS_MODEL_RunInference(
                    (void *)g_clsfInputData,
                    sizeof(g_clsfInputData),
                    &predClass,
                    &tinf_us,
                    IS_ENABLED(CONFIG_SENSOR_INFENG_VERBOSE));

#ifdef CONFIG_SENSOR_DATA_FORMAT_INTERLEAVED
                memcpy(&g_clsfInputData[0],
                       &g_clsfInputData[CONFIG_CLSF_CHANNELS *
                           (CONFIG_CLSF_WINDOW - CLSF_OFFSET)],
                       CONFIG_CLSF_CHANNELS * CLSF_OFFSET * sizeof(float));
#else
                for (int i = 0; i < CONFIG_CLSF_CHANNELS; i++) {
                    memcpy(&g_clsfInputData[i * CONFIG_CLSF_WINDOW],
                           &g_clsfInputData[i * CONFIG_CLSF_WINDOW +
                               (CONFIG_CLSF_WINDOW - CLSF_OFFSET)],
                           CLSF_OFFSET * sizeof(float));
                }
#endif
                clsfSampIdx = CLSF_OFFSET;

                printk("\rInference: t=%d us, class=%d   ",
                       (int)tinf_us, (int)predClass);
            }
        }
    }
}

K_THREAD_DEFINE(inf_thread, 4096,
                sensor_inf_thread, NULL, NULL, NULL,
                K_PRIO_PREEMPT(2), 0, 0);
#endif /* CONFIG_SENSOR_COLLECT_RUN_INFERENCE */

/* ---- Timer Callback ---- */
static struct sensor_data s_sensorData = { .sampleNum = 0 };
static uint8_t  s_first_run = 1;
static uint64_t s_t0 = 0;

static void sensor_timer_cb(struct k_timer *timer)
{
    if (s_first_run) {
        s_first_run = 0;
        s_t0 = (uint64_t)TIMER_GetTimeInUS();
        return;
    }

    SENSOR_Run(s_sensorData.rawDataSensor);
    s_sensorData.sampleNum++;
    s_sensorData.ts_us = (uint64_t)TIMER_GetTimeInUS() - s_t0;

    if (k_msgq_put(&g_sensorCollectQueue, &s_sensorData, K_NO_WAIT) != 0) {
        LOG_WRN("Queue full, sample dropped");
        s_sensorData.sampleNum--;
    }
}

/* ---- MainTask ---- */
void MainTask(void)
{
    int rc = SENSOR_Init();
    if (rc != 0) {
        LOG_ERR("SENSOR_Init failed: %d", rc);
        return;
    }

    k_timer_init(&g_sensorCollectTimer, sensor_timer_cb, NULL);
    k_timer_start(&g_sensorCollectTimer,
                  K_MSEC(1000 / CONFIG_SENSOR_COLLECT_RATE_HZ),
                  K_MSEC(1000 / CONFIG_SENSOR_COLLECT_RATE_HZ));

    LOG_INF("MainTask running, rate=%d Hz", CONFIG_SENSOR_COLLECT_RATE_HZ);
}