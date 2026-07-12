#include "u585_board.h"
#include "u585_demo.h"

typedef struct
{
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t ism330dhcx_driver_ready;
} U585_Exp19State;

volatile U585_Exp19State g_u585_exp19_state;

static void exp19_init(void)
{
  U585_Board_InitBasicGpio();
  g_u585_exp19_state.iterations = 0U;
  g_u585_exp19_state.tick_ms = HAL_GetTick();
  g_u585_exp19_state.ism330dhcx_driver_ready = 0U;
}

static void exp19_loop(void)
{
  g_u585_exp19_state.iterations++;
  g_u585_exp19_state.tick_ms = HAL_GetTick();
  U585_Board_ToggleGreenLed();
  HAL_Delay(1000U);
}

const U585_Demo U585_Demo_Exp19 = {
  "19",
  "ISM330DHCX IMU experiment shell",
  exp19_init,
  exp19_loop,
};
