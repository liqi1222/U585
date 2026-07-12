#include "u585_board.h"
#include "u585_demo.h"

typedef struct
{
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t rtc_nonsecure_ready;
} U585_Exp08State;

volatile U585_Exp08State g_u585_exp08_state;

static void exp08_init(void)
{
  U585_Board_InitBasicGpio();
  g_u585_exp08_state.iterations = 0U;
  g_u585_exp08_state.tick_ms = HAL_GetTick();
  g_u585_exp08_state.rtc_nonsecure_ready = 0U;
}

static void exp08_loop(void)
{
  g_u585_exp08_state.iterations++;
  g_u585_exp08_state.tick_ms = HAL_GetTick();
  U585_Board_ToggleGreenLed();
  HAL_Delay(1000U);
}

const U585_Demo U585_Demo_Exp08 = {
  "08",
  "RTC wakeup and backup-domain experiment shell",
  exp08_init,
  exp08_loop,
};
