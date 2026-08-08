#ifndef U585_GPDMA_H
#define U585_GPDMA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

extern DMA_HandleTypeDef hdma_gpdma1_ch0_ns;
extern DMA_HandleTypeDef hdma_gpdma1_ch1_adc_ns;
extern DMA_HandleTypeDef hdma_gpdma1_ch2_dac_ns;

HAL_StatusTypeDef U585_GPDMA_Init(void);
uint8_t U585_GPDMA_IsReady(void);
HAL_StatusTypeDef U585_GPDMA_MemCopyWords(const uint32_t *src, uint32_t *dst, uint32_t word_count);
HAL_StatusTypeDef U585_GPDMA_InitAdcCircular(void);
HAL_StatusTypeDef U585_GPDMA_InitDacCircular(void);

#ifdef __cplusplus
}
#endif

#endif /* U585_GPDMA_H */
