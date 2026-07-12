#include "u585_board.h"
#include "u585_demo.h"

typedef struct
{
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t st25dv_driver_ready;
} U585_Exp25State;

volatile U585_Exp25State g_u585_exp25_state;

static void exp25_init(void)
{
  U585_Board_InitBasicGpio();
  g_u585_exp25_state.iterations = 0U;
  g_u585_exp25_state.tick_ms = HAL_GetTick();
  g_u585_exp25_state.st25dv_driver_ready = 0U;
}

static void exp25_loop(void)
{
  g_u585_exp25_state.iterations++;
  g_u585_exp25_state.tick_ms = HAL_GetTick();
  U585_Board_ToggleGreenLed();
  HAL_Delay(1000U);
}

const U585_Demo U585_Demo_Exp25 = {
  "25",
  "ST25DV dynamic NFC tag experiment shell",
  exp25_init,
  exp25_loop,
};
