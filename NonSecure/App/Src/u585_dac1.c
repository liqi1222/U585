#include "u585_dac1.h"
#include "u585_gpdma.h"

DAC_HandleTypeDef hdac1_ns;
static uint8_t u585_dac1_ready = 0U;

void HAL_DAC_MspInit(DAC_HandleTypeDef *dacHandle)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  if (dacHandle->Instance != DAC1)
  {
    return;
  }

  __HAL_RCC_DAC1_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  HAL_PWREx_EnableVddA();

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADCDAC;
  PeriphClkInit.AdcDacClockSelection = RCC_ADCDACCLKSOURCE_HCLK;
  (void)HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

  /* PA4 = DAC1_OUT1 (also STMOD SPI1_NSS on B-U585I-IOT02A). */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void HAL_DAC_MspDeInit(DAC_HandleTypeDef *dacHandle)
{
  if (dacHandle->Instance != DAC1)
  {
    return;
  }

  __HAL_RCC_DAC1_CLK_DISABLE();
  HAL_GPIO_DeInit(GPIOA, GPIO_PIN_4);
}

static HAL_StatusTypeDef u585_dac1_init(uint32_t trigger, uint8_t start_channel)
{
  DAC_ChannelConfTypeDef sConfig = {0};

  u585_dac1_ready = 0U;

  hdac1_ns.Instance = DAC1;
  if (HAL_DAC_Init(&hdac1_ns) != HAL_OK)
  {
    return HAL_ERROR;
  }

  sConfig.DAC_HighFrequency = DAC_HIGH_FREQUENCY_INTERFACE_MODE_AUTOMATIC;
  sConfig.DAC_DMADoubleDataMode = DISABLE;
  sConfig.DAC_SignedFormat = DISABLE;
  sConfig.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;
  sConfig.DAC_Trigger = trigger;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  sConfig.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_EXTERNAL;
  sConfig.DAC_UserTrimming = DAC_TRIMMING_FACTORY;
  if (HAL_DAC_ConfigChannel(&hdac1_ns, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    return HAL_ERROR;
  }

  if ((start_channel != 0U) && (HAL_DAC_Start(&hdac1_ns, DAC_CHANNEL_1) != HAL_OK))
  {
    return HAL_ERROR;
  }

  u585_dac1_ready = 1U;
  return HAL_OK;
}

HAL_StatusTypeDef U585_DAC1_Init(void)
{
  return u585_dac1_init(DAC_TRIGGER_NONE, 1U);
}

HAL_StatusTypeDef U585_DAC1_InitTimerDma(void)
{
  if (u585_dac1_init(DAC_TRIGGER_T2_TRGO, 0U) != HAL_OK)
  {
    return HAL_ERROR;
  }
  if (U585_GPDMA_InitDacCircular() != HAL_OK)
  {
    u585_dac1_ready = 0U;
    return HAL_ERROR;
  }

  __HAL_LINKDMA(&hdac1_ns, DMA_Handle1, hdma_gpdma1_ch2_dac_ns);
  HAL_NVIC_SetPriority(GPDMA1_Channel2_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(GPDMA1_Channel2_IRQn);
  HAL_NVIC_SetPriority(DAC1_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(DAC1_IRQn);
  return HAL_OK;
}

HAL_StatusTypeDef U585_DAC1_StartTimerDma(const uint32_t *samples, uint32_t sample_count)
{
  if ((u585_dac1_ready == 0U) || (samples == NULL) || (sample_count == 0U))
  {
    return HAL_ERROR;
  }
  if (HAL_DAC_SetValue(&hdac1_ns, DAC_CHANNEL_1, DAC_ALIGN_12B_R, samples[0]) != HAL_OK)
  {
    return HAL_ERROR;
  }
  return HAL_DAC_Start_DMA(&hdac1_ns,
                           DAC_CHANNEL_1,
                           (const uint32_t *)samples,
                           sample_count,
                           DAC_ALIGN_12B_R);
}

HAL_StatusTypeDef U585_DAC1_StopTimerDma(void)
{
  if (u585_dac1_ready == 0U)
  {
    return HAL_ERROR;
  }
  return HAL_DAC_Stop_DMA(&hdac1_ns, DAC_CHANNEL_1);
}

uint8_t U585_DAC1_IsReady(void)
{
  return u585_dac1_ready;
}

HAL_StatusTypeDef U585_DAC1_SetCode12(uint32_t code)
{
  if (u585_dac1_ready == 0U)
  {
    return HAL_ERROR;
  }
  if (code > 4095U)
  {
    code = 4095U;
  }
  return HAL_DAC_SetValue(&hdac1_ns, DAC_CHANNEL_1, DAC_ALIGN_12B_R, code);
}

uint32_t U585_DAC1_GetDor(void)
{
  if (u585_dac1_ready == 0U)
  {
    return 0U;
  }
  return (DAC1->DOR1 & 0xFFFU);
}
