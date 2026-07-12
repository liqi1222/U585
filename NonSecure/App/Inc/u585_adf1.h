#ifndef U585_ADF1_H
#define U585_ADF1_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

extern MDF_HandleTypeDef hadf1_ns;

HAL_StatusTypeDef U585_ADF1_Init(void);
uint8_t U585_ADF1_IsReady(void);
HAL_StatusTypeDef U585_ADF1_ReadSample(int32_t *sample_out, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* U585_ADF1_H */
