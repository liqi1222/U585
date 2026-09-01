#include "u585_board.h"
#include "u585_demo.h"
#include "u585_fault.h"
#include "u585_log.h"

#ifndef U585_EXP04_ENABLE_FAULT_TRIGGER
#define U585_EXP04_ENABLE_FAULT_TRIGGER 0
#endif

typedef struct
{
  uint32_t magic;
  uint32_t init_count;
  uint32_t loop_count;
  uint32_t tick_ms;
  uint32_t button_events;
  uint32_t itm_enabled;
  uint32_t fault_trigger_enabled;
  uint32_t fault_armed;
} U585_Exp04State;

volatile U585_Exp04State g_u585_exp04_state;

static void exp04_itm_puts(const char *text)
{
  if ((CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk) == 0U)
  {
    return;
  }

  while (*text != '\0')
  {
    (void)ITM_SendChar((uint32_t)*text);
    text++;
  }
}

static void exp04_trigger_fault(void)
{
#if (U585_EXP04_ENABLE_FAULT_TRIGGER != 0)
  /* UsageFault：Thumb 未定义指令（NonSecure 内可控） */
  __asm volatile(".hword 0xDE00");
#else
  U585_Log_WriteLine("[U585][04] fault trigger disabled (set U585_EXP04_ENABLE_FAULT_TRIGGER=1)");
#endif
}

static void exp04_init(void)
{
  U585_Board_InitBasicGpio();
  U585_Fault_EnableConfigurableFaults();

  g_u585_exp04_state.magic = 0xA5850004UL;
  g_u585_exp04_state.init_count++;
  g_u585_exp04_state.loop_count = 0U;
  g_u585_exp04_state.tick_ms = HAL_GetTick();
  g_u585_exp04_state.button_events = 0U;
  g_u585_exp04_state.itm_enabled =
      ((CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk) != 0U) ? 1U : 0U;
  g_u585_exp04_state.fault_trigger_enabled = (uint32_t)U585_EXP04_ENABLE_FAULT_TRIGGER;
  g_u585_exp04_state.fault_armed = 0U;

  U585_Log_WriteLine("");
  U585_Log_WriteLine("[U585][04] SWD/SWO/Fault demo start");
  U585_Log_WriteLine("[U585][04] VCP=USART1 NonSecure direct; ITM/SWO optional");
  U585_Log_WriteU32("[U585][04] itm_trcena=", g_u585_exp04_state.itm_enabled);
  U585_Log_WriteU32("[U585][04] fault_trigger_enabled=", g_u585_exp04_state.fault_trigger_enabled);
  exp04_itm_puts("U585 exp04 debug/fault demo\r\n");
}

static void exp04_loop(void)
{
  static uint8_t last_button = 0U;
  uint8_t button = U585_Board_IsUserButtonPressed();

  g_u585_exp04_state.loop_count++;
  g_u585_exp04_state.tick_ms = HAL_GetTick();

  U585_Board_ToggleGreenLed();

  if ((button != 0U) && (last_button == 0U))
  {
    g_u585_exp04_state.button_events++;
    g_u585_exp04_state.fault_armed = 1U;
    U585_Log_WriteU32("[U585][04] button_edge=", g_u585_exp04_state.button_events);
    exp04_itm_puts("U585 exp04 button event\r\n");
    exp04_trigger_fault();
    g_u585_exp04_state.fault_armed = 0U;
  }
  last_button = button;

  if ((g_u585_exp04_state.loop_count % 20U) == 0U)
  {
    U585_Log_WriteU32("[U585][04] heartbeat=", g_u585_exp04_state.loop_count);
  }

#if (U585_EXP04_ENABLE_FAULT_TRIGGER != 0)
  if (g_u585_exp04_state.loop_count == 8U)
  {
    U585_Log_WriteLine("[U585][04] auto fault trigger (demo capture)");
    exp04_trigger_fault();
  }
#endif

  HAL_Delay(250U);
}

const U585_Demo U585_Demo_Exp04DebugFault = {
  "04",
  "SWD SWO and fault-site debugging",
  exp04_init,
  exp04_loop,
};
