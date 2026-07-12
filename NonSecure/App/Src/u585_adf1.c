#include "u585_adf1.h"

MDF_HandleTypeDef hadf1_ns;
static uint8_t u585_adf1_ready = 0U;

void HAL_MDF_MspInit(MDF_HandleTypeDef *mdfHandle)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  if (!IS_ADF_INSTANCE(mdfHandle->Instance))
  {
    return;
  }

  /* Match ST BSP: PLL3Q ≈ 11.428 MHz → CCK0 ≈ 2.857 MHz with divider 4. */
  PeriphClkInit.PLL3.PLL3Source = RCC_PLLSOURCE_MSI;
  PeriphClkInit.PLL3.PLL3M = 1;
  PeriphClkInit.PLL3.PLL3N = 80;
  PeriphClkInit.PLL3.PLL3P = 28;
  PeriphClkInit.PLL3.PLL3Q = 28;
  PeriphClkInit.PLL3.PLL3R = 2;
  PeriphClkInit.PLL3.PLL3RGE = 0U; /* PLL3 VCI range */
  PeriphClkInit.PLL3.PLL3FRACN = 0;
  PeriphClkInit.PLL3.PLL3ClockOut = RCC_PLL3_DIVQ;
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADF1;
  PeriphClkInit.Adf1ClockSelection = RCC_ADF1CLKSOURCE_PLL3;
  (void)HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

  __HAL_RCC_ADF1_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();

  /* PE9=ADF1_CCK0, PE10=ADF1_SDI0 (board MIC1). */
  GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF3_ADF1;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
}

void HAL_MDF_MspDeInit(MDF_HandleTypeDef *mdfHandle)
{
  if (!IS_ADF_INSTANCE(mdfHandle->Instance))
  {
    return;
  }

  __HAL_RCC_ADF1_CLK_DISABLE();
  HAL_GPIO_DeInit(GPIOE, GPIO_PIN_9 | GPIO_PIN_10);
}

HAL_StatusTypeDef U585_ADF1_Init(void)
{
  u585_adf1_ready = 0U;

  hadf1_ns.Instance = ADF1_Filter0;
  hadf1_ns.Init.CommonParam.InterleavedFilters = 0U;
  hadf1_ns.Init.CommonParam.ProcClockDivider = 1U;
  hadf1_ns.Init.CommonParam.OutputClock.Activation = ENABLE;
  hadf1_ns.Init.CommonParam.OutputClock.Pins = MDF_OUTPUT_CLOCK_0;
  hadf1_ns.Init.CommonParam.OutputClock.Divider = 4U;
  hadf1_ns.Init.CommonParam.OutputClock.Trigger.Activation = DISABLE;
  hadf1_ns.Init.CommonParam.OutputClock.Trigger.Source = MDF_CLOCK_TRIG_TRGO;
  hadf1_ns.Init.CommonParam.OutputClock.Trigger.Edge = MDF_CLOCK_TRIG_RISING_EDGE;
  hadf1_ns.Init.SerialInterface.Activation = ENABLE;
  hadf1_ns.Init.SerialInterface.Mode = MDF_SITF_NORMAL_SPI_MODE;
  hadf1_ns.Init.SerialInterface.ClockSource = MDF_SITF_CCK0_SOURCE;
  hadf1_ns.Init.SerialInterface.Threshold = 31U;
  hadf1_ns.Init.FilterBistream = MDF_BITSTREAM5_RISING;

  if (HAL_MDF_Init(&hadf1_ns) != HAL_OK)
  {
    return HAL_ERROR;
  }

  u585_adf1_ready = 1U;
  return HAL_OK;
}

uint8_t U585_ADF1_IsReady(void)
{
  return u585_adf1_ready;
}

HAL_StatusTypeDef U585_ADF1_ReadSample(int32_t *sample_out, uint32_t timeout_ms)
{
  MDF_FilterConfigTypeDef filterConfig = {0};

  if ((u585_adf1_ready == 0U) || (sample_out == NULL))
  {
    return HAL_ERROR;
  }

  filterConfig.DataSource = MDF_DATA_SOURCE_BSMX;
  filterConfig.Delay = 0U;
  filterConfig.CicMode = MDF_ONE_FILTER_SINC4;
  filterConfig.DecimationRatio = 24U;
  filterConfig.Offset = 0;
  filterConfig.Gain = 0;
  filterConfig.ReshapeFilter.Activation = DISABLE;
  filterConfig.ReshapeFilter.DecimationRatio = MDF_RSF_DECIMATION_RATIO_4;
  filterConfig.HighPassFilter.Activation = DISABLE;
  filterConfig.HighPassFilter.CutOffFrequency = MDF_HPF_CUTOFF_0_000625FPCM;
  filterConfig.Integrator.Activation = DISABLE;
  filterConfig.Integrator.Value = 4U;
  filterConfig.Integrator.OutputDivision = MDF_INTEGRATOR_OUTPUT_NO_DIV;
  filterConfig.SoundActivity.Activation = DISABLE;
  filterConfig.AcquisitionMode = MDF_MODE_ASYNC_CONT;
  filterConfig.FifoThreshold = MDF_FIFO_THRESHOLD_NOT_EMPTY;
  filterConfig.DiscardSamples = 1U;
  filterConfig.Trigger.Source = MDF_FILTER_TRIG_TRGO;
  filterConfig.Trigger.Edge = MDF_FILTER_TRIG_RISING_EDGE;
  filterConfig.SnapshotFormat = MDF_SNAPSHOT_23BITS;

  if (HAL_MDF_AcqStart(&hadf1_ns, &filterConfig) != HAL_OK)
  {
    return HAL_ERROR;
  }

  if (HAL_MDF_PollForAcq(&hadf1_ns, timeout_ms) != HAL_OK)
  {
    (void)HAL_MDF_AcqStop(&hadf1_ns);
    return HAL_ERROR;
  }

  if (HAL_MDF_GetAcqValue(&hadf1_ns, sample_out) != HAL_OK)
  {
    (void)HAL_MDF_AcqStop(&hadf1_ns);
    return HAL_ERROR;
  }

  (void)HAL_MDF_AcqStop(&hadf1_ns);
  return HAL_OK;
}
