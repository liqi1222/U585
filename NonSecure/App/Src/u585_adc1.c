#include "u585_adc1.h"
#include "u585_gpdma.h"

ADC_HandleTypeDef hadc1_ns;
static uint8_t u585_adc1_ready = 0U;

void HAL_ADC_MspInit(ADC_HandleTypeDef *adcHandle)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  if (adcHandle->Instance != ADC1)
  {
    return;
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADCDAC;
  PeriphClkInit.AdcDacClockSelection = RCC_ADCDACCLKSOURCE_HCLK;
  (void)HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

  __HAL_RCC_ADC12_CLK_ENABLE();
  HAL_PWREx_EnableVddA();
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef *adcHandle)
{
  if (adcHandle->Instance != ADC1)
  {
    return;
  }

  __HAL_RCC_ADC12_CLK_DISABLE();
}

static HAL_StatusTypeDef u585_adc1_init(uint32_t trigger,
                                         uint32_t trigger_edge,
                                         uint32_t conversion_data_management,
                                         uint32_t dma_continuous_requests)
{
  ADC_ChannelConfTypeDef sConfig = {0};

  u585_adc1_ready = 0U;

  hadc1_ns.Instance = ADC1;
  hadc1_ns.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV4;
  hadc1_ns.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1_ns.Init.GainCompensation = 0;
  hadc1_ns.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1_ns.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1_ns.Init.LowPowerAutoWait = DISABLE;
  hadc1_ns.Init.ContinuousConvMode = DISABLE;
  hadc1_ns.Init.NbrOfConversion = 1;
  hadc1_ns.Init.DiscontinuousConvMode = DISABLE;
  hadc1_ns.Init.ExternalTrigConv = trigger;
  hadc1_ns.Init.ExternalTrigConvEdge = trigger_edge;
  hadc1_ns.Init.DMAContinuousRequests = dma_continuous_requests;
  hadc1_ns.Init.TriggerFrequencyMode = ADC_TRIGGER_FREQ_HIGH;
  hadc1_ns.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
  hadc1_ns.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
  hadc1_ns.Init.ConversionDataManagement = conversion_data_management;
  hadc1_ns.Init.OversamplingMode = DISABLE;

  if (HAL_ADC_Init(&hadc1_ns) != HAL_OK)
  {
    return HAL_ERROR;
  }

  if (HAL_ADCEx_Calibration_Start(&hadc1_ns, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED) != HAL_OK)
  {
    return HAL_ERROR;
  }

  /* Default channel config; callers may reconfigure before each read. */
  sConfig.Channel = ADC_CHANNEL_VREFINT;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_814CYCLES;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1_ns, &sConfig) != HAL_OK)
  {
    return HAL_ERROR;
  }

  u585_adc1_ready = 1U;
  return HAL_OK;
}

HAL_StatusTypeDef U585_ADC1_Init(void)
{
  return u585_adc1_init(ADC_SOFTWARE_START,
                         ADC_EXTERNALTRIGCONVEDGE_NONE,
                         ADC_CONVERSIONDATA_DR,
                         DISABLE);
}

HAL_StatusTypeDef U585_ADC1_InitTimerDma(void)
{
  if (u585_adc1_init(ADC_EXTERNALTRIG_T2_TRGO,
                     ADC_EXTERNALTRIGCONVEDGE_RISING,
                     ADC_CONVERSIONDATA_DMA_CIRCULAR,
                     ENABLE) != HAL_OK)
  {
    return HAL_ERROR;
  }
  if (U585_GPDMA_InitAdcCircular() != HAL_OK)
  {
    u585_adc1_ready = 0U;
    return HAL_ERROR;
  }

  __HAL_LINKDMA(&hadc1_ns, DMA_Handle, hdma_gpdma1_ch1_adc_ns);
  HAL_NVIC_SetPriority(GPDMA1_Channel1_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(GPDMA1_Channel1_IRQn);
  HAL_NVIC_SetPriority(ADC1_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(ADC1_IRQn);
  return HAL_OK;
}

HAL_StatusTypeDef U585_ADC1_StartTimerDma(uint16_t *samples, uint32_t sample_count)
{
  if ((u585_adc1_ready == 0U) || (samples == NULL) || (sample_count == 0U))
  {
    return HAL_ERROR;
  }
  return HAL_ADC_Start_DMA(&hadc1_ns, (const uint32_t *)samples, sample_count);
}

HAL_StatusTypeDef U585_ADC1_StopTimerDma(void)
{
  if (u585_adc1_ready == 0U)
  {
    return HAL_ERROR;
  }
  return HAL_ADC_Stop_DMA(&hadc1_ns);
}

uint8_t U585_ADC1_IsReady(void)
{
  return u585_adc1_ready;
}

HAL_StatusTypeDef U585_ADC1_ReadChannel(uint32_t channel, uint32_t *raw_out)
{
  ADC_ChannelConfTypeDef sConfig = {0};

  if ((u585_adc1_ready == 0U) || (raw_out == NULL))
  {
    return HAL_ERROR;
  }

  sConfig.Channel = channel;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_814CYCLES;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1_ns, &sConfig) != HAL_OK)
  {
    return HAL_ERROR;
  }

  if (HAL_ADC_Start(&hadc1_ns) != HAL_OK)
  {
    return HAL_ERROR;
  }
  if (HAL_ADC_PollForConversion(&hadc1_ns, 50U) != HAL_OK)
  {
    (void)HAL_ADC_Stop(&hadc1_ns);
    return HAL_ERROR;
  }

  *raw_out = HAL_ADC_GetValue(&hadc1_ns);
  (void)HAL_ADC_Stop(&hadc1_ns);
  return HAL_OK;
}
