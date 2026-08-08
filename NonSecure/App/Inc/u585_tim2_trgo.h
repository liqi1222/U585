#ifndef U585_TIM2_TRGO_H
#define U585_TIM2_TRGO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define U585_TIM2_TRGO_HZ (10000U)

extern TIM_HandleTypeDef htim2_ns;

HAL_StatusTypeDef U585_TIM2_TRGO_Init(void);
HAL_StatusTypeDef U585_TIM2_TRGO_Start(void);
HAL_StatusTypeDef U585_TIM2_TRGO_Stop(void);
uint32_t U585_TIM2_TRGO_GetHz(void);

#ifdef __cplusplus
}
#endif

#endif /* U585_TIM2_TRGO_H */
