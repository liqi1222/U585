#include "u585_board.h"
#include "u585_demo.h"

typedef struct
{
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t network_stack_ready;
} U585_Exp29State;

volatile U585_Exp29State g_u585_exp29_state;

static void exp29_init(void)
{
  U585_Board_InitBasicGpio();
  g_u585_exp29_state.iterations = 0U;
  g_u585_exp29_state.tick_ms = HAL_GetTick();
  g_u585_exp29_state.network_stack_ready = 0U;
}

static void exp29_loop(void)
{
  g_u585_exp29_state.iterations++;
  g_u585_exp29_state.tick_ms = HAL_GetTick();
  U585_Board_ToggleGreenLed();
  HAL_Delay(1000U);
}

const U585_Demo U585_Demo_Exp29 = {
  "29",
  "MQTT cloud connection experiment shell",
  exp29_init,
  exp29_loop,
};
