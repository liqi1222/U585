#include "u585_board.h"
#include "u585_demo.h"

typedef struct
{
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t ospi_flash_ready;
} U585_Exp23State;

volatile U585_Exp23State g_u585_exp23_state;

static void exp23_init(void)
{
  U585_Board_InitBasicGpio();
  g_u585_exp23_state.iterations = 0U;
  g_u585_exp23_state.tick_ms = HAL_GetTick();
  g_u585_exp23_state.ospi_flash_ready = 0U;
}

static void exp23_loop(void)
{
  g_u585_exp23_state.iterations++;
  g_u585_exp23_state.tick_ms = HAL_GetTick();
  U585_Board_ToggleGreenLed();
  HAL_Delay(1000U);
}

const U585_Demo U585_Demo_Exp23 = {
  "23",
  "OctoSPI Flash and memory-mapped XIP shell",
  exp23_init,
  exp23_loop,
};
