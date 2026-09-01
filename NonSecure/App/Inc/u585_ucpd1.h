#ifndef U585_UCPD1_H
#define U585_UCPD1_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

HAL_StatusTypeDef U585_UCPD1_Init(void);
uint8_t U585_UCPD1_IsReady(void);
uint32_t U585_UCPD1_GetCc1State(void);
uint32_t U585_UCPD1_GetCc2State(void);
uint32_t U585_UCPD1_GetSr(void);

#ifdef __cplusplus
}
#endif

#endif /* U585_UCPD1_H */
