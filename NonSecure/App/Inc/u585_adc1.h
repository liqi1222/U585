#ifndef U585_ADC1_H
#define U585_ADC1_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

extern ADC_HandleTypeDef hadc1_ns;

HAL_StatusTypeDef U585_ADC1_Init(void);
uint8_t U585_ADC1_IsReady(void);
HAL_StatusTypeDef U585_ADC1_ReadChannel(uint32_t channel, uint32_t *raw_out);
HAL_StatusTypeDef U585_ADC1_InitTimerDma(void);
HAL_StatusTypeDef U585_ADC1_StartTimerDma(uint16_t *samples, uint32_t sample_count);
HAL_StatusTypeDef U585_ADC1_StopTimerDma(void);

#ifdef __cplusplus
}
#endif

#endif /* U585_ADC1_H */
