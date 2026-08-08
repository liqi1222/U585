#include "u585_tim2_trgo.h"

#define U585_TIM2_COUNTER_HZ (10000000U)

TIM_HandleTypeDef htim2_ns;
static uint8_t u585_tim2_trgo_ready = 0U;

HAL_StatusTypeDef U585_TIM2_TRGO_Init(void)
{
  TIM_MasterConfigTypeDef master_config = {0};

  u585_tim2_trgo_ready = 0U;
  __HAL_RCC_TIM2_CLK_ENABLE();

  htim2_ns.Instance = TIM2;
  htim2_ns.Init.Prescaler = (SystemCoreClock / U585_TIM2_COUNTER_HZ) - 1U;
  htim2_ns.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2_ns.Init.Period = (U585_TIM2_COUNTER_HZ / U585_TIM2_TRGO_HZ) - 1U;
  htim2_ns.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2_ns.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2_ns) != HAL_OK)
  {
    return HAL_ERROR;
  }

  master_config.MasterOutputTrigger = TIM_TRGO_UPDATE;
  master_config.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  master_config.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2_ns, &master_config) != HAL_OK)
  {
    return HAL_ERROR;
  }

  u585_tim2_trgo_ready = 1U;
  return HAL_OK;
}

HAL_StatusTypeDef U585_TIM2_TRGO_Start(void)
{
  if (u585_tim2_trgo_ready == 0U)
  {
    return HAL_ERROR;
  }
  return HAL_TIM_Base_Start(&htim2_ns);
}

HAL_StatusTypeDef U585_TIM2_TRGO_Stop(void)
{
  if (u585_tim2_trgo_ready == 0U)
  {
    return HAL_ERROR;
  }
  return HAL_TIM_Base_Stop(&htim2_ns);
}

uint32_t U585_TIM2_TRGO_GetHz(void)
{
  return U585_TIM2_TRGO_HZ;
}
