#include "u585_board.h"
#include "u585_demo.h"

typedef struct
{
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t hts221_driver_ready;
} U585_Exp17State;

volatile U585_Exp17State g_u585_exp17_state;

static void exp17_init(void)
{
  U585_Board_InitBasicGpio();
  g_u585_exp17_state.iterations = 0U;
  g_u585_exp17_state.tick_ms = HAL_GetTick();
  g_u585_exp17_state.hts221_driver_ready = 0U;
}

static void exp17_loop(void)
{
  g_u585_exp17_state.iterations++;
  g_u585_exp17_state.tick_ms = HAL_GetTick();
  U585_Board_ToggleGreenLed();
  HAL_Delay(1000U);
}

const U585_Demo U585_Demo_Exp17 = {
  "17",
  "HTS221 temperature/humidity sensor experiment shell",
  exp17_init,
  exp17_loop,
};
