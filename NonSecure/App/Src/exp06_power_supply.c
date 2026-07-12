#include "u585_board.h"
#include "u585_demo.h"

typedef struct
{
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t requires_power_measurement;
} U585_Exp06State;

volatile U585_Exp06State g_u585_exp06_state;

static void exp06_init(void)
{
  U585_Board_InitBasicGpio();
  g_u585_exp06_state.iterations = 0U;
  g_u585_exp06_state.tick_ms = HAL_GetTick();
  g_u585_exp06_state.requires_power_measurement = 1U;
}

static void exp06_loop(void)
{
  g_u585_exp06_state.iterations++;
  g_u585_exp06_state.tick_ms = HAL_GetTick();
  U585_Board_ToggleGreenLed();
  HAL_Delay(1000U);
}

const U585_Demo U585_Demo_Exp06 = {
  "06",
  "Power architecture: SMPS/LDO and voltage scale",
  exp06_init,
  exp06_loop,
};
