#include "u585_board.h"
#include "u585_demo.h"

typedef struct
{
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t i2c_nonsecure_ready;
} U585_Exp12State;

volatile U585_Exp12State g_u585_exp12_state;

static void exp12_init(void)
{
  U585_Board_InitBasicGpio();
  g_u585_exp12_state.iterations = 0U;
  g_u585_exp12_state.tick_ms = HAL_GetTick();
  g_u585_exp12_state.i2c_nonsecure_ready = 0U;
}

static void exp12_loop(void)
{
  g_u585_exp12_state.iterations++;
  g_u585_exp12_state.tick_ms = HAL_GetTick();
  U585_Board_ToggleGreenLed();
  HAL_Delay(1000U);
}

const U585_Demo U585_Demo_Exp12 = {
  "12",
  "I2C sensor bus experiment shell",
  exp12_init,
  exp12_loop,
};
