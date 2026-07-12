#include "u585_board.h"
#include "u585_demo.h"

typedef struct
{
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t vl53l5cx_driver_ready;
} U585_Exp21State;

volatile U585_Exp21State g_u585_exp21_state;

static void exp21_init(void)
{
  U585_Board_InitBasicGpio();
  g_u585_exp21_state.iterations = 0U;
  g_u585_exp21_state.tick_ms = HAL_GetTick();
  g_u585_exp21_state.vl53l5cx_driver_ready = 0U;
}

static void exp21_loop(void)
{
  g_u585_exp21_state.iterations++;
  g_u585_exp21_state.tick_ms = HAL_GetTick();
  U585_Board_ToggleGreenLed();
  HAL_Delay(1000U);
}

const U585_Demo U585_Demo_Exp21 = {
  "21",
  "VL53L5CX multi-zone ToF experiment shell",
  exp21_init,
  exp21_loop,
};
