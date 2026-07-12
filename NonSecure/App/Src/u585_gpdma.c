#include "u585_gpdma.h"

DMA_HandleTypeDef hdma_gpdma1_ch0_ns;
static uint8_t u585_gpdma_ready = 0U;

HAL_StatusTypeDef U585_GPDMA_Init(void)
{
  u585_gpdma_ready = 0U;

  __HAL_RCC_GPDMA1_CLK_ENABLE();

  hdma_gpdma1_ch0_ns.Instance = GPDMA1_Channel0;
  hdma_gpdma1_ch0_ns.Init.Request = DMA_REQUEST_SW;
  hdma_gpdma1_ch0_ns.Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
  hdma_gpdma1_ch0_ns.Init.Direction = DMA_MEMORY_TO_MEMORY;
  hdma_gpdma1_ch0_ns.Init.SrcInc = DMA_SINC_INCREMENTED;
  hdma_gpdma1_ch0_ns.Init.DestInc = DMA_DINC_INCREMENTED;
  hdma_gpdma1_ch0_ns.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_WORD;
  hdma_gpdma1_ch0_ns.Init.DestDataWidth = DMA_DEST_DATAWIDTH_WORD;
  hdma_gpdma1_ch0_ns.Init.Priority = DMA_LOW_PRIORITY_HIGH_WEIGHT;
  hdma_gpdma1_ch0_ns.Init.SrcBurstLength = 1;
  hdma_gpdma1_ch0_ns.Init.DestBurstLength = 1;
  hdma_gpdma1_ch0_ns.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT0;
  hdma_gpdma1_ch0_ns.Init.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
  hdma_gpdma1_ch0_ns.Init.Mode = DMA_NORMAL;

  if (HAL_DMA_Init(&hdma_gpdma1_ch0_ns) != HAL_OK)
  {
    return HAL_ERROR;
  }

  u585_gpdma_ready = 1U;
  return HAL_OK;
}

uint8_t U585_GPDMA_IsReady(void)
{
  return u585_gpdma_ready;
}

HAL_StatusTypeDef U585_GPDMA_MemCopyWords(const uint32_t *src, uint32_t *dst, uint32_t word_count)
{
  HAL_StatusTypeDef st;
  uint32_t byte_count;
  uint32_t tickstart;

  if ((u585_gpdma_ready == 0U) || (src == NULL) || (dst == NULL) || (word_count == 0U))
  {
    return HAL_ERROR;
  }

  if (hdma_gpdma1_ch0_ns.State != HAL_DMA_STATE_READY)
  {
    hdma_gpdma1_ch0_ns.State = HAL_DMA_STATE_READY;
    __HAL_UNLOCK(&hdma_gpdma1_ch0_ns);
  }

  byte_count = word_count * 4U;

  __HAL_DMA_CLEAR_FLAG(&hdma_gpdma1_ch0_ns,
                       DMA_FLAG_TC | DMA_FLAG_HT | DMA_FLAG_DTE | DMA_FLAG_ULE | DMA_FLAG_USE | DMA_FLAG_SUSP | DMA_FLAG_TO);
  if ((hdma_gpdma1_ch0_ns.Instance->CSR & DMA_CSR_TCF) != 0U)
  {
    return HAL_ERROR;
  }

  st = HAL_DMA_Start(&hdma_gpdma1_ch0_ns,
                     (uint32_t)src,
                     (uint32_t)dst,
                     byte_count);
  if (st != HAL_OK)
  {
    return st;
  }

  /* Ensure software request is armed after addresses/size are programmed. */
  hdma_gpdma1_ch0_ns.Instance->CTR2 |= DMA_CTR2_SWREQ;

  tickstart = HAL_GetTick();
  while ((hdma_gpdma1_ch0_ns.Instance->CSR & DMA_CSR_TCF) == 0U)
  {
    if ((hdma_gpdma1_ch0_ns.Instance->CSR & (DMA_CSR_DTEF | DMA_CSR_ULEF | DMA_CSR_USEF)) != 0U)
    {
      hdma_gpdma1_ch0_ns.Instance->CCR |= DMA_CCR_RESET;
      hdma_gpdma1_ch0_ns.State = HAL_DMA_STATE_READY;
      __HAL_UNLOCK(&hdma_gpdma1_ch0_ns);
      return HAL_ERROR;
    }
    if ((HAL_GetTick() - tickstart) > 100U)
    {
      (void)HAL_DMA_Abort(&hdma_gpdma1_ch0_ns);
      return HAL_ERROR;
    }
  }

  __HAL_DMA_CLEAR_FLAG(&hdma_gpdma1_ch0_ns, DMA_FLAG_TC | DMA_FLAG_HT);
  hdma_gpdma1_ch0_ns.State = HAL_DMA_STATE_READY;
  __HAL_UNLOCK(&hdma_gpdma1_ch0_ns);
  return HAL_OK;
}
