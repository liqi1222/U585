#include "u585_emw3080.h"

static uint8_t u585_emw_ready = 0U;

HAL_StatusTypeDef U585_EMW3080_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  u585_emw_ready = 0U;

  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /* Chip_En / WKUP_W: output, pulse low then high to boot module. */
  GPIO_InitStruct.Pin = U585_EMW_CHIP_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(U585_EMW_CHIP_EN_GPIO_Port, &GPIO_InitStruct);

  /* FLOW / NOTIFY: inputs from module. */
  GPIO_InitStruct.Pin = U585_EMW_FLOW_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(U585_EMW_FLOW_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = U585_EMW_NOTIFY_Pin;
  HAL_GPIO_Init(U585_EMW_NOTIFY_GPIO_Port, &GPIO_InitStruct);

  /* Reset pulse: hold Chip_En low, then release high (module enable). */
  HAL_GPIO_WritePin(U585_EMW_CHIP_EN_GPIO_Port, U585_EMW_CHIP_EN_Pin, GPIO_PIN_RESET);
  HAL_Delay(50U);
  HAL_GPIO_WritePin(U585_EMW_CHIP_EN_GPIO_Port, U585_EMW_CHIP_EN_Pin, GPIO_PIN_SET);

  /* Allow module firmware to boot; ST examples wait ~1 s before HCI. */
  HAL_Delay(1200U);

  u585_emw_ready = 1U;
  return HAL_OK;
}

uint8_t U585_EMW3080_IsReady(void)
{
  return u585_emw_ready;
}

uint8_t U585_EMW3080_ReadFlow(void)
{
  return (HAL_GPIO_ReadPin(U585_EMW_FLOW_GPIO_Port, U585_EMW_FLOW_Pin) == GPIO_PIN_SET) ? 1U : 0U;
}

uint8_t U585_EMW3080_ReadNotify(void)
{
  return (HAL_GPIO_ReadPin(U585_EMW_NOTIFY_GPIO_Port, U585_EMW_NOTIFY_Pin) == GPIO_PIN_SET) ? 1U : 0U;
}

uint8_t U585_EMW3080_ReadChipEn(void)
{
  return (HAL_GPIO_ReadPin(U585_EMW_CHIP_EN_GPIO_Port, U585_EMW_CHIP_EN_Pin) == GPIO_PIN_SET) ? 1U : 0U;
}
