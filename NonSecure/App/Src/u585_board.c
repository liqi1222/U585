#include "u585_board.h"

void U585_Board_InitBasicGpio(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  HAL_GPIO_WritePin(U585_LED_RED_PORT, U585_LED_RED_PIN, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(U585_LED_GREEN_PORT, U585_LED_GREEN_PIN, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = U585_LED_RED_PIN | U585_LED_GREEN_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = U585_USER_BUTTON_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(U585_USER_BUTTON_PORT, &GPIO_InitStruct);
}

void U585_Board_SetRedLed(GPIO_PinState state)
{
  HAL_GPIO_WritePin(U585_LED_RED_PORT, U585_LED_RED_PIN, state);
}

void U585_Board_SetGreenLed(GPIO_PinState state)
{
  HAL_GPIO_WritePin(U585_LED_GREEN_PORT, U585_LED_GREEN_PIN, state);
}

void U585_Board_ToggleRedLed(void)
{
  HAL_GPIO_TogglePin(U585_LED_RED_PORT, U585_LED_RED_PIN);
}

void U585_Board_ToggleGreenLed(void)
{
  HAL_GPIO_TogglePin(U585_LED_GREEN_PORT, U585_LED_GREEN_PIN);
}

GPIO_PinState U585_Board_ReadUserButton(void)
{
  return HAL_GPIO_ReadPin(U585_USER_BUTTON_PORT, U585_USER_BUTTON_PIN);
}

uint8_t U585_Board_IsUserButtonPressed(void)
{
  return (U585_Board_ReadUserButton() == U585_BUTTON_PRESSED_STATE) ? 1U : 0U;
}
