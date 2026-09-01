#include "u585_i2c2.h"

I2C_HandleTypeDef hi2c2_ns;
static uint8_t u585_i2c2_ready = 0U;

void HAL_I2C_MspInit(I2C_HandleTypeDef *i2cHandle)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  if (i2cHandle->Instance != I2C2)
  {
    return;
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C2;
  PeriphClkInit.I2c2ClockSelection = RCC_I2C2CLKSOURCE_PCLK1;
  (void)HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_I2C2_CLK_ENABLE();

  GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef *i2cHandle)
{
  if (i2cHandle->Instance != I2C2)
  {
    return;
  }

  __HAL_RCC_I2C2_CLK_DISABLE();
  HAL_GPIO_DeInit(GPIOH, GPIO_PIN_4 | GPIO_PIN_5);
}

HAL_StatusTypeDef U585_I2C2_Init(void)
{
  u585_i2c2_ready = 0U;

  hi2c2_ns.Instance = I2C2;
  hi2c2_ns.Init.Timing = 0x30909DEC;
  hi2c2_ns.Init.OwnAddress1 = 0;
  hi2c2_ns.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2_ns.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2_ns.Init.OwnAddress2 = 0;
  hi2c2_ns.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2_ns.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2_ns.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

  if (HAL_I2C_Init(&hi2c2_ns) != HAL_OK)
  {
    return HAL_ERROR;
  }
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2_ns, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    return HAL_ERROR;
  }
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2_ns, 0) != HAL_OK)
  {
    return HAL_ERROR;
  }

  u585_i2c2_ready = 1U;
  return HAL_OK;
}

uint8_t U585_I2C2_IsReady(void)
{
  return u585_i2c2_ready;
}
