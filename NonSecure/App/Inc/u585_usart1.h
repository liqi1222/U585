#ifndef U585_USART1_H
#define U585_USART1_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

extern UART_HandleTypeDef huart1_ns;

HAL_StatusTypeDef U585_USART1_Init(void);
uint8_t U585_USART1_IsReady(void);

#ifdef __cplusplus
}
#endif

#endif /* U585_USART1_H */
