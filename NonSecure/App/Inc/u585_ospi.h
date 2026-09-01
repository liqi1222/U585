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

/* Octal DTR (8D-8D-8D) + memory-mapped support, exp33 */
HAL_StatusTypeDef U585_OSPI_Flash_EnterOctalDtr(void);
uint8_t U585_OSPI_Flash_IsOctalReady(void);
uint32_t U585_OSPI_Flash_OctalFailStep(void);
void U585_OSPI_Flash_OctalDiag(uint32_t out[5]);
uint32_t U585_OSPI_Flash_MmFailStep(void);
uint32_t U585_OSPI_Psram_MmFailStep(void);
void U585_OSPI_DlybDiag(uint32_t out[3]);
HAL_StatusTypeDef U585_OSPI_Flash_EnableMemoryMapped(void);
HAL_StatusTypeDef U585_OSPI_Flash_DisableMemoryMapped(void);
HAL_StatusTypeDef U585_OSPI_Flash_Erase4K(uint32_t addr);
HAL_StatusTypeDef U585_OSPI_Flash_WriteEnableOctal(void);
HAL_StatusTypeDef U585_OSPI_Flash_PageProgram(uint32_t addr, const uint8_t *data, uint32_t size);
HAL_StatusTypeDef U585_OSPI_Flash_ReadRaw(uint32_t addr, uint8_t *data, uint32_t size);

HAL_StatusTypeDef U585_OSPI_Psram_Init(void);
uint8_t U585_OSPI_Psram_IsReady(void);
HAL_StatusTypeDef U585_OSPI_Psram_MemTest(uint32_t *wrote, uint32_t *readback);

/* Octal DTR + memory-mapped support, exp33 */
HAL_StatusTypeDef U585_OSPI_Psram_EnterOctalDtr(void);
uint8_t U585_OSPI_Psram_IsOctalReady(void);
HAL_StatusTypeDef U585_OSPI_Psram_EnableMemoryMapped(void);
HAL_StatusTypeDef U585_OSPI_Psram_DisableMemoryMapped(void);

#ifdef __cplusplus
}
#endif

#endif /* U585_OSPI_H */
