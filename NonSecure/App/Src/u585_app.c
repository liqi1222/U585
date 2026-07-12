#include "u585_app.h"
#include "u585_board.h"
#include "u585_usart1.h"

#ifndef U585_ACTIVE_DEMO
#define U585_ACTIVE_DEMO 3
#endif

#if (U585_ACTIVE_DEMO == 3)
static const U585_Demo *active_demo = &U585_Demo_Exp03LedButton;
#elif (U585_ACTIVE_DEMO == 4)
static const U585_Demo *active_demo = &U585_Demo_Exp04DebugFault;
#elif (U585_ACTIVE_DEMO == 5)
static const U585_Demo *active_demo = &U585_Demo_Exp05ClockTree;
#elif (U585_ACTIVE_DEMO == 6)
static const U585_Demo *active_demo = &U585_Demo_Exp06;
#elif (U585_ACTIVE_DEMO == 7)
static const U585_Demo *active_demo = &U585_Demo_Exp07;
#elif (U585_ACTIVE_DEMO == 8)
static const U585_Demo *active_demo = &U585_Demo_Exp08;
#elif (U585_ACTIVE_DEMO == 9)
static const U585_Demo *active_demo = &U585_Demo_Exp09;
#elif (U585_ACTIVE_DEMO == 10)
static const U585_Demo *active_demo = &U585_Demo_Exp10;
#elif (U585_ACTIVE_DEMO == 11)
static const U585_Demo *active_demo = &U585_Demo_Exp11;
#elif (U585_ACTIVE_DEMO == 12)
static const U585_Demo *active_demo = &U585_Demo_Exp12;
#elif (U585_ACTIVE_DEMO == 13)
static const U585_Demo *active_demo = &U585_Demo_Exp13;
#elif (U585_ACTIVE_DEMO == 14)
static const U585_Demo *active_demo = &U585_Demo_Exp14;
#elif (U585_ACTIVE_DEMO == 15)
static const U585_Demo *active_demo = &U585_Demo_Exp15;
#elif (U585_ACTIVE_DEMO == 16)
static const U585_Demo *active_demo = &U585_Demo_Exp16;
#elif (U585_ACTIVE_DEMO == 17)
static const U585_Demo *active_demo = &U585_Demo_Exp17;
#elif (U585_ACTIVE_DEMO == 18)
static const U585_Demo *active_demo = &U585_Demo_Exp18;
#elif (U585_ACTIVE_DEMO == 19)
static const U585_Demo *active_demo = &U585_Demo_Exp19;
#elif (U585_ACTIVE_DEMO == 20)
static const U585_Demo *active_demo = &U585_Demo_Exp20;
#elif (U585_ACTIVE_DEMO == 21)
static const U585_Demo *active_demo = &U585_Demo_Exp21;
#elif (U585_ACTIVE_DEMO == 22)
static const U585_Demo *active_demo = &U585_Demo_Exp22;
#elif (U585_ACTIVE_DEMO == 23)
static const U585_Demo *active_demo = &U585_Demo_Exp23;
#elif (U585_ACTIVE_DEMO == 24)
static const U585_Demo *active_demo = &U585_Demo_Exp24;
#elif (U585_ACTIVE_DEMO == 25)
static const U585_Demo *active_demo = &U585_Demo_Exp25;
#elif (U585_ACTIVE_DEMO == 26)
static const U585_Demo *active_demo = &U585_Demo_Exp26;
#elif (U585_ACTIVE_DEMO == 27)
static const U585_Demo *active_demo = &U585_Demo_Exp27;
#elif (U585_ACTIVE_DEMO == 28)
static const U585_Demo *active_demo = &U585_Demo_Exp28;
#elif (U585_ACTIVE_DEMO == 29)
static const U585_Demo *active_demo = &U585_Demo_Exp29;
#elif (U585_ACTIVE_DEMO == 30)
static const U585_Demo *active_demo = &U585_Demo_Exp30;
#elif (U585_ACTIVE_DEMO == 31)
static const U585_Demo *active_demo = &U585_Demo_Exp31;
#elif (U585_ACTIVE_DEMO == 32)
static const U585_Demo *active_demo = &U585_Demo_Exp32;
#else
static const U585_Demo *active_demo = &U585_Demo_Unsupported;
#endif

static void unsupported_init(void)
{
}

static void unsupported_loop(void)
{
}

const U585_Demo U585_Demo_Unsupported = {
  "unsupported",
  "Unsupported U585 demo selection",
  unsupported_init,
  unsupported_loop,
};

void U585_App_Init(void)
{
  /* Blink red once if USART1 NS init fails (keeps board diagnosable without VCP). */
  if (U585_USART1_Init() != HAL_OK)
  {
    U585_Board_InitBasicGpio();
    U585_Board_SetRedLed(GPIO_PIN_SET);
  }

  if ((active_demo != 0) && (active_demo->init != 0))
  {
    active_demo->init();
  }
}

void U585_App_Loop(void)
{
  if ((active_demo != 0) && (active_demo->loop != 0))
  {
    active_demo->loop();
  }
}

const U585_Demo *U585_App_CurrentDemo(void)
{
  return active_demo;
}
