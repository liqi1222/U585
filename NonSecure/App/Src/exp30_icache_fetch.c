#include "u585_board.h"
#include "u585_demo.h"

typedef struct
{
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t icache_nonsecure_ready;
} U585_Exp30State;

volatile U585_Exp30State g_u585_exp30_state;

static void exp30_init(void)
{
  U585_Board_InitBasicGpio();
  g_u585_exp30_state.iterations = 0U;
  g_u585_exp30_state.tick_ms = HAL_GetTick();
  g_u585_exp30_state.icache_nonsecure_ready = 0U;
}

static void exp30_loop(void)
{
  g_u585_exp30_state.iterations++;
  g_u585_exp30_state.tick_ms = HAL_GetTick();
  U585_Board_ToggleGreenLed();
  HAL_Delay(1000U);
}

const U585_Demo U585_Demo_Exp30 = {
  "30",
  "ICACHE and instruction-fetch acceleration shell",
  exp30_init,
  exp30_loop,
};
