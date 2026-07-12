#include "u585_usart1.h"

UART_HandleTypeDef huart1_ns;
static uint8_t u585_usart1_ready = 0U;

void HAL_UART_MspInit(UART_HandleTypeDef *uartHandle)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  if (uartHandle->Instance != USART1)
  {
    return;
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  (void)HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

  __HAL_RCC_USART1_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *uartHandle)
{
  if (uartHandle->Instance != USART1)
  {
    return;
  }

  __HAL_RCC_USART1_CLK_DISABLE();
  HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9 | GPIO_PIN_10);
}

HAL_StatusTypeDef U585_USART1_Init(void)
{
  HAL_StatusTypeDef status;

  u585_usart1_ready = 0U;

  huart1_ns.Instance = USART1;
  huart1_ns.Init.BaudRate = 115200;
  huart1_ns.Init.WordLength = UART_WORDLENGTH_8B;
  huart1_ns.Init.StopBits = UART_STOPBITS_1;
  huart1_ns.Init.Parity = UART_PARITY_NONE;
  huart1_ns.Init.Mode = UART_MODE_TX_RX;
  huart1_ns.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1_ns.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1_ns.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1_ns.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1_ns.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

  status = HAL_UART_Init(&huart1_ns);
  if (status != HAL_OK)
  {
    return status;
  }

  u585_usart1_ready = 1U;
  return HAL_OK;
}

uint8_t U585_USART1_IsReady(void)
{
  return u585_usart1_ready;
}
