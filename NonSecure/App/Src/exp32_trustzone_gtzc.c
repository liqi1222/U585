#include "u585_board.h"
#include "u585_demo.h"
#include "u585_log.h"
#include "secure_nsc.h"

typedef struct
{
  uint32_t magic;
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t tz_ready;
  uint32_t nsc_ok;
  uint32_t sau_regions;
  uint32_t flash_s;
  uint32_t flash_ns;
  uint32_t vtor_ns;
  uint32_t running_ns;
  uint32_t gtzc_handoff;
} U585_Exp32State;

volatile U585_Exp32State g_u585_exp32_state;

static void exp32_init(void)
{
  uint32_t nsc_magic;

  U585_Board_InitBasicGpio();
  g_u585_exp32_state.magic = 0xA5850020UL;
  g_u585_exp32_state.iterations = 0U;
  g_u585_exp32_state.tick_ms = HAL_GetTick();
  g_u585_exp32_state.gtzc_handoff = 1U; /* this project already uses GTZC TZSC handoff */

  U585_Log_WriteLine("");
  U585_Log_WriteLine("[U585][32] TrustZone dual-image observe (CubeMX TZ, not TF-M)");
  U585_Log_WriteLine("[U585][32] probe NSC veneer + SAU/Flash map");

  /* NS image is linked at 0x08100000; Secure jumps here after GTZC setup. */
  g_u585_exp32_state.running_ns = 1U;
  g_u585_exp32_state.vtor_ns = SCB->VTOR;

  nsc_magic = SECURE_GetTzMagic();
  g_u585_exp32_state.nsc_ok = (nsc_magic == 0xA5850032UL) ? 1U : 0U;
  g_u585_exp32_state.sau_regions = SECURE_GetSauRegionCount();
  g_u585_exp32_state.flash_s = SECURE_GetFlashSecureBase();
  g_u585_exp32_state.flash_ns = SECURE_GetFlashNonSecureBase();
  g_u585_exp32_state.tz_ready =
      ((g_u585_exp32_state.nsc_ok != 0U) &&
       (g_u585_exp32_state.flash_s == 0x0C000000UL) &&
       (g_u585_exp32_state.flash_ns == 0x08100000UL)) ? 1U : 0U;

  U585_Log_WriteU32("[U585][32] tz_ready=", g_u585_exp32_state.tz_ready);
  U585_Log_WriteU32("[U585][32] nsc_ok=", g_u585_exp32_state.nsc_ok);
  U585_Log_WriteU32("[U585][32] sau_regions=", g_u585_exp32_state.sau_regions);
  U585_Log_WriteU32("[U585][32] flash_s=", g_u585_exp32_state.flash_s);
  U585_Log_WriteU32("[U585][32] flash_ns=", g_u585_exp32_state.flash_ns);
  U585_Log_WriteU32("[U585][32] vtor_ns=", g_u585_exp32_state.vtor_ns);
  U585_Log_WriteU32("[U585][32] running_ns=", g_u585_exp32_state.running_ns);
  U585_Log_WriteU32("[U585][32] gtzc_handoff=", g_u585_exp32_state.gtzc_handoff);
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
  "TrustZone / GTZC partition observe",
  exp32_init,
  exp32_loop,
};
