#include "u585_board.h"
#include "u585_demo.h"
#include "u585_log.h"

#ifndef U585_EXP07_ENABLE_SLEEP
#define U585_EXP07_ENABLE_SLEEP 1
#endif

typedef struct
{
  uint32_t magic;
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t sleep_entry_requests;
  uint32_t sleep_completed;
  uint32_t stop_standby_enabled;
} U585_Exp07State;

volatile U585_Exp07State g_u585_exp07_state;

static void exp07_enter_sleep_briefly(void)
{
#if (U585_EXP07_ENABLE_SLEEP != 0)
  /* Short Sleep only: keeps SWD alive better than Stop/Standby. */
  HAL_SuspendTick();
  HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
  HAL_ResumeTick();
  g_u585_exp07_state.sleep_completed++;
#else
  U585_Log_WriteLine("[U585][07] sleep disabled");
#endif
}

static void exp07_init(void)
{
  U585_Board_InitBasicGpio();
  g_u585_exp07_state.magic = 0xA5850007UL;
  g_u585_exp07_state.iterations = 0U;
  g_u585_exp07_state.tick_ms = HAL_GetTick();
  g_u585_exp07_state.sleep_entry_requests = 0U;
  g_u585_exp07_state.sleep_completed = 0U;
  g_u585_exp07_state.stop_standby_enabled = 0U;

  U585_Log_WriteLine("");
  U585_Log_WriteLine("[U585][07] low-power hooks start");
  U585_Log_WriteLine("[U585][07] button edge -> short Sleep(WFI); Stop/Standby deferred");
  U585_Log_WriteU32("[U585][07] sleep_enabled=", (uint32_t)U585_EXP07_ENABLE_SLEEP);
}

static void exp07_loop(void)
{
  static uint8_t last_button = 0U;
  uint8_t button = U585_Board_IsUserButtonPressed();

  g_u585_exp07_state.iterations++;
  g_u585_exp07_state.tick_ms = HAL_GetTick();

  if ((button != 0U) && (last_button == 0U))
  {
    g_u585_exp07_state.sleep_entry_requests++;
    U585_Log_WriteU32("[U585][07] sleep_request=", g_u585_exp07_state.sleep_entry_requests);
    U585_Board_SetRedLed(GPIO_PIN_SET);
    exp07_enter_sleep_briefly();
    U585_Board_SetRedLed(GPIO_PIN_RESET);
    U585_Log_WriteU32("[U585][07] sleep_completed=", g_u585_exp07_state.sleep_completed);
  }
  last_button = button;

  U585_Board_ToggleGreenLed();
  HAL_Delay(500U);
}

const U585_Demo U585_Demo_Exp07 = {
  "07",
  "Low-power modes and current measurement hooks",
  exp07_init,
  exp07_loop,
};
