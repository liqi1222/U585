#include "u585_board.h"
#include "u585_demo.h"
#include "u585_log.h"

TIM_HandleTypeDef htim2_ns;

typedef struct
{
  uint32_t magic;
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t tim_ready;
  uint32_t pwm_duty_percent;
  uint32_t update_irq_count;
} U585_Exp10State;

volatile U585_Exp10State g_u585_exp10_state;

void U585_Exp10_OnTimUpdate(void)
{
  static uint32_t phase = 0U;
  g_u585_exp10_state.update_irq_count++;
  phase++;

  /* Software PWM on green LED using TIM2 update ticks (period ~1ms). */
  {
    uint32_t slot = phase % 100U;
    if (slot < g_u585_exp10_state.pwm_duty_percent)
    {
      U585_Board_SetGreenLed(GPIO_PIN_SET);
    }
    else
    {
      U585_Board_SetGreenLed(GPIO_PIN_RESET);
    }
  }
}

static HAL_StatusTypeDef exp10_tim_init(void)
{
  __HAL_RCC_TIM2_CLK_ENABLE();

  htim2_ns.Instance = TIM2;
  htim2_ns.Init.Prescaler = (SystemCoreClock / 1000000U) - 1U; /* 1 MHz */
  htim2_ns.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2_ns.Init.Period = 999U; /* 1 kHz update */
  htim2_ns.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2_ns.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2_ns) != HAL_OK)
  {
    return HAL_ERROR;
  }

  HAL_NVIC_SetPriority(TIM2_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(TIM2_IRQn);

  if (HAL_TIM_Base_Start_IT(&htim2_ns) != HAL_OK)
  {
    return HAL_ERROR;
  }

  return HAL_OK;
}

static void exp10_init(void)
{
  U585_Board_InitBasicGpio();
  g_u585_exp10_state.magic = 0xA585000AUL;
  g_u585_exp10_state.iterations = 0U;
  g_u585_exp10_state.tick_ms = HAL_GetTick();
  g_u585_exp10_state.pwm_duty_percent = 20U;
  g_u585_exp10_state.update_irq_count = 0U;
  g_u585_exp10_state.tim_ready = (exp10_tim_init() == HAL_OK) ? 1U : 0U;

  U585_Log_WriteLine("");
  U585_Log_WriteLine("[U585][10] TIM2 software-PWM on green LED");
  U585_Log_WriteU32("[U585][10] tim_ready=", g_u585_exp10_state.tim_ready);
  U585_Log_WriteU32("[U585][10] duty_percent=", g_u585_exp10_state.pwm_duty_percent);
  U585_Log_WriteLine("[U585][10] NOTE: scope waveform / input-capture -> author follow-up");
}

static void exp10_loop(void)
{
  static uint8_t last_button = 0U;
  uint8_t button = U585_Board_IsUserButtonPressed();

  g_u585_exp10_state.iterations++;
  g_u585_exp10_state.tick_ms = HAL_GetTick();

  if ((button != 0U) && (last_button == 0U))
  {
    g_u585_exp10_state.pwm_duty_percent += 20U;
    if (g_u585_exp10_state.pwm_duty_percent > 80U)
    {
      g_u585_exp10_state.pwm_duty_percent = 20U;
    }
    U585_Log_WriteU32("[U585][10] duty_percent=", g_u585_exp10_state.pwm_duty_percent);
  }
  last_button = button;

  if ((g_u585_exp10_state.iterations % 5U) == 0U)
  {
    U585_Log_WriteU32("[U585][10] tim_irq=", g_u585_exp10_state.update_irq_count);
  }

  HAL_Delay(200U);
}

const U585_Demo U585_Demo_Exp10 = {
  "10",
  "Timer PWM and input-capture experiment",
  exp10_init,
  exp10_loop,
};
