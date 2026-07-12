#include "u585_board.h"
#include "u585_demo.h"

typedef struct
{
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t ospi_psram_ready;
} U585_Exp24State;

volatile U585_Exp24State g_u585_exp24_state;

static void exp24_init(void)
{
  U585_Board_InitBasicGpio();
  g_u585_exp24_state.iterations = 0U;
  g_u585_exp24_state.tick_ms = HAL_GetTick();
  g_u585_exp24_state.ospi_psram_ready = 0U;
}

static void exp24_loop(void)
{
  g_u585_exp24_state.iterations++;
  g_u585_exp24_state.tick_ms = HAL_GetTick();
  U585_Board_ToggleGreenLed();
  HAL_Delay(1000U);
}

const U585_Demo U585_Demo_Exp24 = {
  "24",
  "OctoSPI PSRAM cache experiment shell",
  exp24_init,
  exp24_loop,
};
