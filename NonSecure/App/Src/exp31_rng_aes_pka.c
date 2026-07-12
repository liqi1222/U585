#include "u585_board.h"
#include "u585_demo.h"

typedef struct
{
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t crypto_peripherals_ready;
} U585_Exp31State;

volatile U585_Exp31State g_u585_exp31_state;

static void exp31_init(void)
{
  U585_Board_InitBasicGpio();
  g_u585_exp31_state.iterations = 0U;
  g_u585_exp31_state.tick_ms = HAL_GetTick();
  g_u585_exp31_state.crypto_peripherals_ready = 0U;
}

static void exp31_loop(void)
{
  g_u585_exp31_state.iterations++;
  g_u585_exp31_state.tick_ms = HAL_GetTick();
  U585_Board_ToggleGreenLed();
  HAL_Delay(1000U);
}

const U585_Demo U585_Demo_Exp31 = {
  "31",
  "RNG AES PKA hardware-crypto experiment shell",
  exp31_init,
  exp31_loop,
};
