#include "u585_board.h"
#include "u585_demo.h"

typedef struct
{
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t lps22hh_driver_ready;
} U585_Exp18State;

volatile U585_Exp18State g_u585_exp18_state;

static void exp18_init(void)
{
  U585_Board_InitBasicGpio();
  g_u585_exp18_state.iterations = 0U;
  g_u585_exp18_state.tick_ms = HAL_GetTick();
  g_u585_exp18_state.lps22hh_driver_ready = 0U;
}

static void exp18_loop(void)
{
  g_u585_exp18_state.iterations++;
  g_u585_exp18_state.tick_ms = HAL_GetTick();
  U585_Board_ToggleGreenLed();
  HAL_Delay(1000U);
}

const U585_Demo U585_Demo_Exp18 = {
  "18",
  "LPS22HH pressure sensor experiment shell",
  exp18_init,
  exp18_loop,
};
