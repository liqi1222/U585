#include "u585_board.h"
#include "u585_demo.h"

typedef struct
{
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t wifi_module_ready;
} U585_Exp27State;

volatile U585_Exp27State g_u585_exp27_state;

static void exp27_init(void)
{
  U585_Board_InitBasicGpio();
  g_u585_exp27_state.iterations = 0U;
  g_u585_exp27_state.tick_ms = HAL_GetTick();
  g_u585_exp27_state.wifi_module_ready = 0U;
}

static void exp27_loop(void)
{
  g_u585_exp27_state.iterations++;
  g_u585_exp27_state.tick_ms = HAL_GetTick();
  U585_Board_ToggleGreenLed();
  HAL_Delay(1000U);
}

const U585_Demo U585_Demo_Exp27 = {
  "27",
  "MXCHIP EMW3080 Wi-Fi bring-up shell",
  exp27_init,
  exp27_loop,
};
