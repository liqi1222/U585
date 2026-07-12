#include "u585_board.h"
#include "u585_demo.h"

#ifndef U585_EXP04_ENABLE_FAULT_TRIGGER
#define U585_EXP04_ENABLE_FAULT_TRIGGER 0
#endif

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
  volatile uint32_t *bad_address = (uint32_t *)0xFFFFFFF0UL;
  volatile uint32_t value = *bad_address;
  (void)value;
#endif
}

static void exp04_init(void)
{
  U585_Board_InitBasicGpio();
  exp04_itm_puts("U585 exp04 debug/fault demo\r\n");
}

static void exp04_loop(void)
{
  U585_Board_ToggleGreenLed();
  HAL_Delay(250U);

  if (U585_Board_IsUserButtonPressed() != 0U)
  {
    exp04_itm_puts("U585 exp04 button event\r\n");
    exp04_trigger_fault();
  }
}

const U585_Demo U585_Demo_Exp04DebugFault = {
  "04",
  "SWD SWO and fault-site debugging",
  exp04_init,
  exp04_loop,
};
