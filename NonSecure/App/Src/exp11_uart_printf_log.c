#include "u585_board.h"
#include "u585_demo.h"

typedef struct
{
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t uart_nonsecure_ready;
} U585_Exp11State;

volatile U585_Exp11State g_u585_exp11_state;

static void exp11_init(void)
{
  U585_Board_InitBasicGpio();
  g_u585_exp11_state.iterations = 0U;
  g_u585_exp11_state.tick_ms = HAL_GetTick();
  g_u585_exp11_state.uart_nonsecure_ready = 0U;
}

static void exp11_loop(void)
{
  g_u585_exp11_state.iterations++;
  g_u585_exp11_state.tick_ms = HAL_GetTick();
  U585_Board_ToggleGreenLed();
  HAL_Delay(1000U);
}

const U585_Demo U585_Demo_Exp11 = {
  "11",
  "UART and printf redirection experiment shell",
  exp11_init,
  exp11_loop,
};
