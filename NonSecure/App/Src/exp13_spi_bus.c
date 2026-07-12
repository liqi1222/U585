#include "u585_board.h"
#include "u585_demo.h"

typedef struct
{
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t spi_nonsecure_ready;
} U585_Exp13State;

volatile U585_Exp13State g_u585_exp13_state;

static void exp13_init(void)
{
  U585_Board_InitBasicGpio();
  g_u585_exp13_state.iterations = 0U;
  g_u585_exp13_state.tick_ms = HAL_GetTick();
  g_u585_exp13_state.spi_nonsecure_ready = 0U;
}

static void exp13_loop(void)
{
  g_u585_exp13_state.iterations++;
  g_u585_exp13_state.tick_ms = HAL_GetTick();
  U585_Board_ToggleGreenLed();
  HAL_Delay(1000U);
}

const U585_Demo U585_Demo_Exp13 = {
  "13",
  "SPI bus experiment shell",
  exp13_init,
  exp13_loop,
};
