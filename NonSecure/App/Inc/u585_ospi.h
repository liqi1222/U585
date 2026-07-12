#ifndef U585_OSPI_H
#define U585_OSPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

extern OSPI_HandleTypeDef hospi_flash_ns;
extern OSPI_HandleTypeDef hospi_psram_ns;

HAL_StatusTypeDef U585_OSPI_Flash_Init(void);
uint8_t U585_OSPI_Flash_IsReady(void);
HAL_StatusTypeDef U585_OSPI_Flash_ReadJedecId(uint8_t id[3]);

HAL_StatusTypeDef U585_OSPI_Psram_Init(void);
uint8_t U585_OSPI_Psram_IsReady(void);
HAL_StatusTypeDef U585_OSPI_Psram_MemTest(uint32_t *wrote, uint32_t *readback);

#ifdef __cplusplus
}
#endif

#endif /* U585_OSPI_H */
