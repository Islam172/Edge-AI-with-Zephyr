#include <zephyr/ztest.h>
#include "inference_engine/emlearn/emlearn_model.h"

ZTEST_SUITE(inference_input_suite, NULL, NULL, NULL, NULL, NULL);

/* NULL Pointer */
ZTEST(inference_input_suite, test_null_input)
{
    int8_t  pred = -1;
    int32_t tinf = 0;

    int rc = EMLEARN_MODEL_RunInference(NULL, 100, &pred, &tinf, 0);
    zassert_equal(rc, -EINVAL,
                  "Expected -EINVAL for NULL input, got %d", rc);
}

/* NULL predClass */
ZTEST(inference_input_suite, test_null_predclass)
{
    float   dummy[384] = {0};
    int32_t tinf = 0;

    int rc = EMLEARN_MODEL_RunInference(dummy, sizeof(dummy), NULL, &tinf, 0);
    zassert_equal(rc, -EINVAL,
                  "Expected -EINVAL for NULL predClass, got %d", rc);
}

/* Zu kleine Puffergröße */
ZTEST(inference_input_suite, test_buffer_too_small)
{
    float   dummy[10] = {0};
    int8_t  pred = -1;
    int32_t tinf = 0;

    int rc = EMLEARN_MODEL_RunInference(dummy, sizeof(dummy), &pred, &tinf, 0);
    zassert_equal(rc, -EMSGSIZE,
                  "Expected -EMSGSIZE for small buffer, got %d", rc);
}

/* Model Init */
ZTEST(inference_input_suite, test_model_init)
{
    int rc = EMLEARN_MODEL_Init();
    zassert_equal(rc, 0,
                  "Expected 0 from EMLEARN_MODEL_Init, got %d", rc);
}