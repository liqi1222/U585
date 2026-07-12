#include "u585_board.h"
#include "u585_demo.h"

typedef struct
{
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t rtos_enabled;
} U585_Exp28State;

volatile U585_Exp28State g_u585_exp28_state;

static void exp28_init(void)
{
  U585_Board_InitBasicGpio();
  g_u585_exp28_state.iterations = 0U;
  g_u585_exp28_state.tick_ms = HAL_GetTick();
  g_u585_exp28_state.rtos_enabled = 0U;
}

static void exp28_loop(void)
{
  g_u585_exp28_state.iterations++;
  g_u585_exp28_state.tick_ms = HAL_GetTick();
  U585_Board_ToggleGreenLed();
  HAL_Delay(1000U);
}

const U585_Demo U585_Demo_Exp28 = {
  "28",
  "FreeRTOS task, queue, and logging shell",
  exp28_init,
  exp28_loop,
};
