#include "u585_board.h"
#include "u585_demo.h"
#include "u585_log.h"

typedef struct
{
  uint32_t magic;
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t button_edges;
  uint32_t press_count;
  uint32_t release_count;
  uint32_t exti_count;
  uint8_t previous_button_state;
  uint8_t exti_enabled;
} U585_Exp09State;

volatile U585_Exp09State g_u585_exp09_state;

void U585_Exp09_OnExti(uint16_t pin)
{
  if (pin != U585_USER_BUTTON_PIN)
  {
    return;
  }

  g_u585_exp09_state.exti_count++;
  if (U585_Board_IsUserButtonPressed() != 0U)
  {
    g_u585_exp09_state.press_count++;
    U585_Board_SetRedLed(GPIO_PIN_SET);
  }
  else
  {
    g_u585_exp09_state.release_count++;
    U585_Board_SetRedLed(GPIO_PIN_RESET);
  }
  g_u585_exp09_state.button_edges =
      g_u585_exp09_state.press_count + g_u585_exp09_state.release_count;
}

static void exp09_enable_exti(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOC_CLK_ENABLE();

  GPIO_InitStruct.Pin = U585_USER_BUTTON_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(U585_USER_BUTTON_PORT, &GPIO_InitStruct);

  HAL_NVIC_SetPriority(EXTI13_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI13_IRQn);
  g_u585_exp09_state.exti_enabled = 1U;
}

static void exp09_init(void)
{
  U585_Board_InitBasicGpio();
  g_u585_exp09_state.magic = 0xA5850009UL;
  g_u585_exp09_state.iterations = 0U;
  g_u585_exp09_state.tick_ms = HAL_GetTick();
  g_u585_exp09_state.button_edges = 0U;
  g_u585_exp09_state.press_count = 0U;
  g_u585_exp09_state.release_count = 0U;
  g_u585_exp09_state.exti_count = 0U;
  g_u585_exp09_state.previous_button_state = U585_Board_IsUserButtonPressed();
  g_u585_exp09_state.exti_enabled = 0U;

  exp09_enable_exti();

  U585_Log_WriteLine("");
  U585_Log_WriteLine("[U585][09] GPIO EXTI demo (PC13 rising/falling)");
  U585_Log_WriteU32("[U585][09] initial_button=", g_u585_exp09_state.previous_button_state);
  U585_Log_WriteU32("[U585][09] exti_enabled=", g_u585_exp09_state.exti_enabled);
}

static void exp09_loop(void)
{
  static uint32_t last_exti_logged = 0U;

  g_u585_exp09_state.iterations++;
  g_u585_exp09_state.tick_ms = HAL_GetTick();

  if (g_u585_exp09_state.exti_count != last_exti_logged)
  {
    last_exti_logged = g_u585_exp09_state.exti_count;
    U585_Log_WriteU32("[U585][09] exti_count=", g_u585_exp09_state.exti_count);
    U585_Log_WriteU32("[U585][09] press=", g_u585_exp09_state.press_count);
    U585_Log_WriteU32("[U585][09] release=", g_u585_exp09_state.release_count);
  }

  if ((g_u585_exp09_state.iterations % 250U) == 0U)
  {
    U585_Board_ToggleGreenLed();
  }

  HAL_Delay(4U);
}

const U585_Demo U585_Demo_Exp09 = {
  "09",
  "GPIO and EXTI external interrupt",
  exp09_init,
  exp09_loop,
};
