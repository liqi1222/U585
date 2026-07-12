#include "u585_board.h"
#include "u585_demo.h"

typedef struct
{
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t tim_nonsecure_ready;
} U585_Exp10State;

volatile U585_Exp10State g_u585_exp10_state;

static void exp10_init(void)
{
  U585_Board_InitBasicGpio();
  g_u585_exp10_state.iterations = 0U;
  g_u585_exp10_state.tick_ms = HAL_GetTick();
  g_u585_exp10_state.tim_nonsecure_ready = 0U;
}

static void exp10_loop(void)
{
  g_u585_exp10_state.iterations++;
  g_u585_exp10_state.tick_ms = HAL_GetTick();
  U585_Board_ToggleGreenLed();
  HAL_Delay(500U);
}

const U585_Demo U585_Demo_Exp10 = {
  "10",
  "Timer PWM and input-capture experiment shell",
  exp10_init,
  exp10_loop,
};
