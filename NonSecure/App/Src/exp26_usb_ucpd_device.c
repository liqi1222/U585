#include "u585_board.h"
#include "u585_demo.h"

typedef struct
{
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t usb_ucpd_ready;
} U585_Exp26State;

volatile U585_Exp26State g_u585_exp26_state;

static void exp26_init(void)
{
  U585_Board_InitBasicGpio();
  g_u585_exp26_state.iterations = 0U;
  g_u585_exp26_state.tick_ms = HAL_GetTick();
  g_u585_exp26_state.usb_ucpd_ready = 0U;
}

static void exp26_loop(void)
{
  g_u585_exp26_state.iterations++;
  g_u585_exp26_state.tick_ms = HAL_GetTick();
  U585_Board_ToggleGreenLed();
  HAL_Delay(1000U);
}

const U585_Demo U585_Demo_Exp26 = {
  "26",
  "USB-C UCPD and USB device experiment shell",
  exp26_init,
  exp26_loop,
};
