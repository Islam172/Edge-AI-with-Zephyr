#ifndef EMLEARN_MODEL_H_
#define EMLEARN_MODEL_H_

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int EMLEARN_MODEL_Init(void);
int EMLEARN_MODEL_RunInference(void *inputData, size_t size,
                               int8_t *predClass, int32_t *tinf_us,
                               uint8_t verbose);

#ifdef __cplusplus
}
#endif

#endif /* EMLEARN_MODEL_H_ */