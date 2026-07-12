#include "u585_board.h"
#include "u585_demo.h"
#include "u585_log.h"
#include "u585_ucpd1.h"

typedef struct
{
  uint32_t magic;
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t ucpd_ready;
  uint32_t cc1;
  uint32_t cc2;
  uint32_t sr;
  uint32_t attached;
} U585_Exp26State;

volatile U585_Exp26State g_u585_exp26_state;

static void exp26_sample(void)
{
  g_u585_exp26_state.cc1 = U585_UCPD1_GetCc1State();
  g_u585_exp26_state.cc2 = U585_UCPD1_GetCc2State();
  g_u585_exp26_state.sr = U585_UCPD1_GetSr();
  /* Sink: non-open CC voltage means a source/Rp is present on that line. */
  g_u585_exp26_state.attached =
      ((g_u585_exp26_state.cc1 != 0U) || (g_u585_exp26_state.cc2 != 0U)) ? 1U : 0U;

  U585_Log_WriteU32("[U585][26] cc1=", g_u585_exp26_state.cc1);
  U585_Log_WriteU32("[U585][26] cc2=", g_u585_exp26_state.cc2);
  U585_Log_WriteU32("[U585][26] attached=", g_u585_exp26_state.attached);
  U585_Log_WriteU32("[U585][26] sr=", g_u585_exp26_state.sr);
}

static void exp26_init(void)
{
  U585_Board_InitBasicGpio();
  g_u585_exp26_state.magic = 0xA585001AUL;
  g_u585_exp26_state.iterations = 0U;
  g_u585_exp26_state.tick_ms = HAL_GetTick();

  U585_Log_WriteLine("");
  U585_Log_WriteLine("[U585][26] UCPD1 Type-C sink CC sense (PA15/PB15)");
  U585_Log_WriteLine("[U585][26] USB FS device stack pending");

  g_u585_exp26_state.ucpd_ready = (U585_UCPD1_Init() == HAL_OK) ? 1U : 0U;
  U585_Log_WriteU32("[U585][26] ucpd_ready=", g_u585_exp26_state.ucpd_ready);

  if (g_u585_exp26_state.ucpd_ready != 0U)
  {
    exp26_sample();
  }
}

static void exp26_loop(void)
{
  g_u585_exp26_state.iterations++;
  g_u585_exp26_state.tick_ms = HAL_GetTick();

  if ((g_u585_exp26_state.ucpd_ready != 0U) && ((g_u585_exp26_state.iterations % 2U) == 0U))
  {
    exp26_sample();
  }

  if ((g_u585_exp26_state.iterations % 4U) == 0U)
  {
    U585_Log_WriteU32("[U585][26] heartbeat=", g_u585_exp26_state.iterations);
  }

  U585_Board_ToggleGreenLed();
  HAL_Delay(500U);
}

const U585_Demo U585_Demo_Exp26 = {
  "26",
  "UCPD1 Type-C CC sense",
  exp26_init,
  exp26_loop,
};
