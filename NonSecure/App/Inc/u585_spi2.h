#ifndef U585_SPI2_H
#define U585_SPI2_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* B-U585I-IOT02A wireless SPI2 (EMW3080 path). */
#ifndef U585_SPI2_SCK_Pin
#define U585_SPI2_SCK_Pin GPIO_PIN_1
#define U585_SPI2_SCK_GPIO_Port GPIOD
#define U585_SPI2_MISO_Pin GPIO_PIN_3
#define U585_SPI2_MISO_GPIO_Port GPIOD
#define U585_SPI2_MOSI_Pin GPIO_PIN_4
#define U585_SPI2_MOSI_GPIO_Port GPIOD
#define U585_SPI2_NSS_Pin GPIO_PIN_12
#define U585_SPI2_NSS_GPIO_Port GPIOB
#endif

extern SPI_HandleTypeDef hspi2_ns;

HAL_StatusTypeDef U585_SPI2_Init(void);
uint8_t U585_SPI2_IsReady(void);
void U585_SPI2_Nss(uint8_t select);
HAL_StatusTypeDef U585_SPI2_Transfer(uint8_t *tx, uint8_t *rx, uint16_t len, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* U585_SPI2_H */
