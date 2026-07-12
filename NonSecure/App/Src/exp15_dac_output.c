#include "u585_board.h"
#include "u585_demo.h"

typedef struct
{
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t dac_nonsecure_ready;
} U585_Exp15State;

volatile U585_Exp15State g_u585_exp15_state;

static void exp15_init(void)
{
  U585_Board_InitBasicGpio();
  g_u585_exp15_state.iterations = 0U;
  g_u585_exp15_state.tick_ms = HAL_GetTick();
  g_u585_exp15_state.dac_nonsecure_ready = 0U;
}

static void exp15_loop(void)
{
  g_u585_exp15_state.iterations++;
  g_u585_exp15_state.tick_ms = HAL_GetTick();
  U585_Board_ToggleGreenLed();
  HAL_Delay(1000U);
}

const U585_Demo U585_Demo_Exp15 = {
  "15",
  "DAC analog-output experiment shell",
  exp15_init,
  exp15_loop,
};
