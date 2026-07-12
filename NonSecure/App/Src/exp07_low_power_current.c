#include "u585_board.h"
#include "u585_demo.h"

typedef struct
{
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t sleep_entry_requests;
} U585_Exp07State;

volatile U585_Exp07State g_u585_exp07_state;

static void exp07_init(void)
{
  U585_Board_InitBasicGpio();
  g_u585_exp07_state.iterations = 0U;
  g_u585_exp07_state.tick_ms = HAL_GetTick();
  g_u585_exp07_state.sleep_entry_requests = 0U;
}

static void exp07_loop(void)
{
  g_u585_exp07_state.iterations++;
  g_u585_exp07_state.tick_ms = HAL_GetTick();

  if (U585_Board_IsUserButtonPressed() != 0U)
  {
    g_u585_exp07_state.sleep_entry_requests++;
  }

  U585_Board_ToggleGreenLed();
  HAL_Delay(1000U);
}

const U585_Demo U585_Demo_Exp07 = {
  "07",
  "Low-power modes and current measurement hooks",
  exp07_init,
  exp07_loop,
};
