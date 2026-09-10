#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <math.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "src/sensor/sensor_collect.h"
#include "inference_engine/emlearn/emlearn_model.h"
#include "inference_engine/emlearn/axis_scaler.h"
#include "inference_engine/emlearn/model.h"

LOG_MODULE_REGISTER(emlearn_model, LOG_LEVEL_INF);

#ifndef RF_THRESHOLD
#define RF_THRESHOLD 0.50f
#endif

static inline float mean_f(const float *x, int n)
{
    double s = 0.0;
    for (int i = 0; i < n; i++) s += x[i];
    return (float)(s / (double)n);
}

static inline float std_f(const float *x, int n)
{
    float mu = mean_f(x, n);
    double s = 0.0;
    for (int i = 0; i < n; i++) {
        double d = (double)x[i] - mu;
        s += d * d;
    }
    return (float)sqrt(s / (double)n);
}

static inline void fetch_norm_sample(const float *in, int s, float out3[3])
{
#ifdef CONFIG_SENSOR_DATA_FORMAT_INTERLEAVED
    out3[0] = (in[CONFIG_CLSF_CHANNELS * s + 0] - AXIS_MEAN[0]) / AXIS_STD[0];
    out3[1] = (in[CONFIG_CLSF_CHANNELS * s + 1] - AXIS_MEAN[1]) / AXIS_STD[1];
    out3[2] = (in[CONFIG_CLSF_CHANNELS * s + 2] - AXIS_MEAN[2]) / AXIS_STD[2];
#else
    out3[0] = (in[0 * CONFIG_CLSF_WINDOW + s] - AXIS_MEAN[0]) / AXIS_STD[0];
    out3[1] = (in[1 * CONFIG_CLSF_WINDOW + s] - AXIS_MEAN[1]) / AXIS_STD[1];
    out3[2] = (in[2 * CONFIG_CLSF_WINDOW + s] - AXIS_MEAN[2]) / AXIS_STD[2];
#endif
}

int EMLEARN_MODEL_Init(void)
{
    return 0;
}

int EMLEARN_MODEL_RunInference(void *inputData, size_t size,
                               int8_t *predClass, int32_t *tinf_us,
                               uint8_t verbose)
{
    if (!inputData || !predClass || !tinf_us) return -EINVAL;

    const size_t need = (size_t)CONFIG_CLSF_CHANNELS *
                        CONFIG_CLSF_WINDOW * sizeof(float);
    if (size < need) {
        if (verbose) {
            LOG_ERR("bad input size: %u < %u",
                    (unsigned)size, (unsigned)need);
        }
        return -EMSGSIZE;
    }

    const float *in = (const float *)inputData;

    static float ax_buf[CONFIG_CLSF_WINDOW];
    static float ay_buf[CONFIG_CLSF_WINDOW];
    static float az_buf[CONFIG_CLSF_WINDOW];

    int64_t t0 = k_uptime_ticks();

    for (int s = 0; s < CONFIG_CLSF_WINDOW; s++) {
        float v[3];
        fetch_norm_sample(in, s, v);
        ax_buf[s] = v[0];
        ay_buf[s] = v[1];
        az_buf[s] = v[2];
    }

    float feat[6];
    feat[0] = mean_f(ax_buf, CONFIG_CLSF_WINDOW);
    feat[1] = std_f(ax_buf,  CONFIG_CLSF_WINDOW);
    feat[2] = mean_f(ay_buf, CONFIG_CLSF_WINDOW);
    feat[3] = std_f(ay_buf,  CONFIG_CLSF_WINDOW);
    feat[4] = mean_f(az_buf, CONFIG_CLSF_WINDOW);
    feat[5] = std_f(az_buf,  CONFIG_CLSF_WINDOW);

    float probs[2] = {0.0f, 0.0f};
    int err = rf_model_predict_proba(feat, 6, probs, 2);

    int64_t t1 = k_uptime_ticks();
    *tinf_us = (int32_t)k_ticks_to_us_near64(t1 - t0);

    if (err != 0) {
        *predClass = (int8_t)rf_model_predict(feat, 6);
    } else {
        *predClass = (probs[1] >= RF_THRESHOLD) ? 1 : 0;
    }

    if (verbose) {
        LOG_INF("p(normal)=%.3f thr=%.2f => %s (%d us)",
                (double)probs[1],
                (double)RF_THRESHOLD,
                *predClass ? "normal" : "ANOMALIE",
                (int)*tinf_us);
        LOG_INF("feat: %.4f %.4f | %.4f %.4f | %.4f %.4f",
                feat[0], feat[1], feat[2],
                feat[3], feat[4], feat[5]);
    }

    return 0;
}