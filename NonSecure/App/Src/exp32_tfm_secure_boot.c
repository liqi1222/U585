#include "u585_board.h"
#include "u585_demo.h"
#include "u585_log.h"
#include "secure_nsc.h"

typedef struct
{
  uint32_t magic;
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t cubemx_tz;
  uint32_t tfm_ready;
  uint32_t sbsfu_ready;
  uint32_t secure_boot_ob;
  uint32_t nsc_ok;
  uint32_t series_end;
} U585_Exp32State;

volatile U585_Exp32State g_u585_exp32_state;

static void exp32_init(void)
{
  U585_Board_InitBasicGpio();
  g_u585_exp32_state.magic = 0xA5850020UL;
  g_u585_exp32_state.iterations = 0U;
  g_u585_exp32_state.tick_ms = HAL_GetTick();

  U585_Log_WriteLine("");
  U585_Log_WriteLine("[U585][32] TF-M / Secure Boot status (read-only)");
  U585_Log_WriteLine("[U585][32] this repo = CubeMX dual-image TZ, NOT TF-M/SBSFU");

  /* Honest inventory: no TF-M BL2 / SBSFU image in this tree. */
  g_u585_exp32_state.cubemx_tz = 1U;
  g_u585_exp32_state.tfm_ready = 0U;
  g_u585_exp32_state.sbsfu_ready = 0U;
  /* Do NOT program option bytes here (irreversible risk). */
  g_u585_exp32_state.secure_boot_ob = 0U;
  g_u585_exp32_state.nsc_ok = (SECURE_GetTzMagic() == 0xA5850032UL) ? 1U : 0U;
  g_u585_exp32_state.series_end = 1U;

  U585_Log_WriteU32("[U585][32] cubemx_tz=", g_u585_exp32_state.cubemx_tz);
  U585_Log_WriteU32("[U585][32] tfm_ready=", g_u585_exp32_state.tfm_ready);
  U585_Log_WriteU32("[U585][32] sbsfu_ready=", g_u585_exp32_state.sbsfu_ready);
  U585_Log_WriteU32("[U585][32] secure_boot_ob=", g_u585_exp32_state.secure_boot_ob);
  U585_Log_WriteU32("[U585][32] nsc_ok=", g_u585_exp32_state.nsc_ok);
  U585_Log_WriteU32("[U585][32] series_end=", g_u585_exp32_state.series_end);
}

static void exp32_loop(void)
{
  g_u585_exp32_state.iterations++;
  g_u585_exp32_state.tick_ms = HAL_GetTick();

  if ((g_u585_exp32_state.iterations % 4U) == 0U)
  {
    U585_Log_WriteU32("[U585][32] heartbeat=", g_u585_exp32_state.iterations);
  }

  U585_Board_ToggleGreenLed();
  HAL_Delay(500U);
}

const U585_Demo U585_Demo_Exp32 = {
  "32",
  "TF-M / Secure Boot inventory stub",
  exp32_init,
  exp32_loop,
};
