#include <zephyr/ztest.h>

/* Definiere Konstanten direkt im Test */
#define CLSF_CHANNELS 3
#define CLSF_WINDOW   128

/* Einfache Validierungsfunktion die wir testen */
static int validate_input(void *inputData, size_t size,
                          int8_t *predClass, int32_t *tinf_us)
{
    if (!inputData || !predClass || !tinf_us) return -EINVAL;

    const size_t need = (size_t)CLSF_CHANNELS * CLSF_WINDOW * sizeof(float);
    if (size < need) return -EMSGSIZE;

    return 0;
}

ZTEST_SUITE(inference_input_suite, NULL, NULL, NULL, NULL, NULL);

ZTEST(inference_input_suite, test_null_input)
{
    int8_t pred = -1;
    int32_t tinf = 0;
    int rc = validate_input(NULL, 100, &pred, &tinf);
    zassert_equal(rc, -EINVAL, "Expected -EINVAL, got %d", rc);
}

ZTEST(inference_input_suite, test_null_predclass)
{
    float dummy[384] = {0};
    int32_t tinf = 0;
    int rc = validate_input(dummy, sizeof(dummy), NULL, &tinf);
    zassert_equal(rc, -EINVAL, "Expected -EINVAL, got %d", rc);
}

ZTEST(inference_input_suite, test_buffer_too_small)
{
    float dummy[10] = {0};
    int8_t pred = -1;
    int32_t tinf = 0;
    int rc = validate_input(dummy, sizeof(dummy), &pred, &tinf);
    zassert_equal(rc, -EMSGSIZE, "Expected -EMSGSIZE, got %d", rc);
}

ZTEST(inference_input_suite, test_valid_input)
{
    float dummy[CLSF_CHANNELS * CLSF_WINDOW] = {0};
    int8_t pred = -1;
    int32_t tinf = 0;
    int rc = validate_input(dummy, sizeof(dummy), &pred, &tinf);
    zassert_equal(rc, 0, "Expected 0, got %d", rc);
}