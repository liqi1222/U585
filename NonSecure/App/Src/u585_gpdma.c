#include "u585_gpdma.h"

DMA_HandleTypeDef hdma_gpdma1_ch0_ns;
DMA_HandleTypeDef hdma_gpdma1_ch1_adc_ns;
DMA_HandleTypeDef hdma_gpdma1_ch2_dac_ns;

static __ALIGNED(32) DMA_NodeTypeDef s_gpdma1_ch1_adc_node;
static __ALIGNED(32) DMA_QListTypeDef s_gpdma1_ch1_adc_queue;
static __ALIGNED(32) DMA_NodeTypeDef s_gpdma1_ch2_dac_node;
static __ALIGNED(32) DMA_QListTypeDef s_gpdma1_ch2_dac_queue;

static uint8_t u585_gpdma_ready = 0U;

static HAL_StatusTypeDef u585_gpdma_init_circular(DMA_HandleTypeDef *hdma,
                                                    DMA_NodeTypeDef *node,
                                                    DMA_QListTypeDef *queue,
                                                    DMA_Channel_TypeDef *instance,
                                                    uint32_t request,
                                                    uint32_t direction,
                                                    uint32_t src_increment,
                                                    uint32_t dst_increment,
                                                    uint32_t src_data_width,
                                                    uint32_t dst_data_width)
{
  DMA_NodeConfTypeDef node_config = {0};

  __HAL_RCC_GPDMA1_CLK_ENABLE();

  hdma->Instance = instance;
  hdma->InitLinkedList.Priority = DMA_HIGH_PRIORITY;
  hdma->InitLinkedList.LinkStepMode = DMA_LSM_FULL_EXECUTION;
  hdma->InitLinkedList.LinkAllocatedPort = DMA_LINK_ALLOCATED_PORT1;
  hdma->InitLinkedList.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
  hdma->InitLinkedList.LinkedListMode = DMA_LINKEDLIST_CIRCULAR;
  if (HAL_DMAEx_List_Init(hdma) != HAL_OK)
  {
    return HAL_ERROR;
  }

  node_config.NodeType = DMA_GPDMA_LINEAR_NODE;
  node_config.Init.Request = request;
  node_config.Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
  node_config.Init.Direction = direction;
  node_config.Init.SrcInc = src_increment;
  node_config.Init.DestInc = dst_increment;
  node_config.Init.SrcDataWidth = src_data_width;
  node_config.Init.DestDataWidth = dst_data_width;
  node_config.Init.Priority = DMA_HIGH_PRIORITY;
  node_config.Init.SrcBurstLength = 1U;
  node_config.Init.DestBurstLength = 1U;
  node_config.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT0;
  node_config.Init.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
  node_config.Init.Mode = DMA_NORMAL;
  node_config.DataHandlingConfig.DataExchange = DMA_EXCHANGE_NONE;
  node_config.DataHandlingConfig.DataAlignment = DMA_DATA_RIGHTALIGN_ZEROPADDED;
  node_config.SrcAddress = 0U;
  node_config.DstAddress = 0U;
  node_config.DataSize = 0U;

  if (HAL_DMAEx_List_BuildNode(&node_config, node) != HAL_OK)
  {
    return HAL_ERROR;
  }
  if (HAL_DMAEx_List_InsertNode_Tail(queue, node) != HAL_OK)
  {
    return HAL_ERROR;
  }
  if (HAL_DMAEx_List_SetCircularMode(queue) != HAL_OK)
  {
    return HAL_ERROR;
  }
  if (HAL_DMAEx_List_LinkQ(hdma, queue) != HAL_OK)
  {
    return HAL_ERROR;
  }

  return HAL_OK;
}

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

HAL_StatusTypeDef U585_GPDMA_InitAdcCircular(void)
{
  return u585_gpdma_init_circular(&hdma_gpdma1_ch1_adc_ns,
                                   &s_gpdma1_ch1_adc_node,
                                   &s_gpdma1_ch1_adc_queue,
                                   GPDMA1_Channel1,
                                   GPDMA1_REQUEST_ADC1,
                                   DMA_PERIPH_TO_MEMORY,
                                   DMA_SINC_FIXED,
                                   DMA_DINC_INCREMENTED,
                                   DMA_SRC_DATAWIDTH_HALFWORD,
                                   DMA_DEST_DATAWIDTH_HALFWORD);
}

HAL_StatusTypeDef U585_GPDMA_InitDacCircular(void)
{
  return u585_gpdma_init_circular(&hdma_gpdma1_ch2_dac_ns,
                                   &s_gpdma1_ch2_dac_node,
                                   &s_gpdma1_ch2_dac_queue,
                                   GPDMA1_Channel2,
                                   GPDMA1_REQUEST_DAC1_CH1,
                                   DMA_MEMORY_TO_PERIPH,
                                   DMA_SINC_INCREMENTED,
                                   DMA_DINC_FIXED,
                                   DMA_SRC_DATAWIDTH_WORD,
                                   DMA_DEST_DATAWIDTH_WORD);
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
