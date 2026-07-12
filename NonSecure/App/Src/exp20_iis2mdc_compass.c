#include "u585_board.h"
#include "u585_demo.h"

typedef struct
{
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t iis2mdc_driver_ready;
} U585_Exp20State;

volatile U585_Exp20State g_u585_exp20_state;

static void exp20_init(void)
{
  U585_Board_InitBasicGpio();
  g_u585_exp20_state.iterations = 0U;
  g_u585_exp20_state.tick_ms = HAL_GetTick();
  g_u585_exp20_state.iis2mdc_driver_ready = 0U;
}

static void exp20_loop(void)
{
  g_u585_exp20_state.iterations++;
  g_u585_exp20_state.tick_ms = HAL_GetTick();
  U585_Board_ToggleGreenLed();
  HAL_Delay(1000U);
}

const U585_Demo U585_Demo_Exp20 = {
  "20",
  "IIS2MDC magnetometer and compass experiment shell",
  exp20_init,
  exp20_loop,
};
