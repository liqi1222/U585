#ifndef U585_EMW3080_H
#define U585_EMW3080_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* B-U585I-IOT02A / UM2839 Table 14 WLAN I/O. */
#define U585_EMW_CHIP_EN_Pin       GPIO_PIN_15
#define U585_EMW_CHIP_EN_GPIO_Port GPIOF
#define U585_EMW_FLOW_Pin          GPIO_PIN_15
#define U585_EMW_FLOW_GPIO_Port    GPIOG
#define U585_EMW_NOTIFY_Pin        GPIO_PIN_14
#define U585_EMW_NOTIFY_GPIO_Port  GPIOD

HAL_StatusTypeDef U585_EMW3080_Init(void);
uint8_t U585_EMW3080_IsReady(void);
uint8_t U585_EMW3080_ReadFlow(void);
uint8_t U585_EMW3080_ReadNotify(void);
uint8_t U585_EMW3080_ReadChipEn(void);

#ifdef __cplusplus
}
#endif

#endif /* U585_EMW3080_H */
