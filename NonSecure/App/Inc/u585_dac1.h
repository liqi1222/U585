#ifndef U585_DAC1_H
#define U585_DAC1_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

extern DAC_HandleTypeDef hdac1_ns;

HAL_StatusTypeDef U585_DAC1_Init(void);
uint8_t U585_DAC1_IsReady(void);
HAL_StatusTypeDef U585_DAC1_SetCode12(uint32_t code);
uint32_t U585_DAC1_GetDor(void);
HAL_StatusTypeDef U585_DAC1_InitTimerDma(void);
HAL_StatusTypeDef U585_DAC1_StartTimerDma(const uint32_t *samples, uint32_t sample_count);
HAL_StatusTypeDef U585_DAC1_StopTimerDma(void);

#ifdef __cplusplus
}
#endif

#endif /* U585_DAC1_H */
