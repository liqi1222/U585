#include "u585_adf1.h"
#include "u585_board.h"
#include "u585_demo.h"
#include "u585_log.h"

typedef struct
{
  uint32_t magic;
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t adf_ready;
  uint32_t sample_ok;
  int32_t sample;
  uint32_t dma_used;
} U585_Exp22State;

volatile U585_Exp22State g_u585_exp22_state;

static void exp22_sample(void)
{
  int32_t sample = 0;

  g_u585_exp22_state.sample_ok = 0U;
  g_u585_exp22_state.sample = 0;
  if (U585_ADF1_ReadSample(&sample, 200U) == HAL_OK)
  {
    g_u585_exp22_state.sample_ok = 1U;
    g_u585_exp22_state.sample = sample;
  }
  U585_Log_WriteU32("[U585][22] sample_ok=", g_u585_exp22_state.sample_ok);
  U585_Log_WriteU32("[U585][22] sample=", (uint32_t)g_u585_exp22_state.sample);
}

static void exp22_init(void)
{
  U585_Board_InitBasicGpio();
  g_u585_exp22_state.magic = 0xA5850016UL;
  g_u585_exp22_state.iterations = 0U;
  g_u585_exp22_state.tick_ms = HAL_GetTick();
  g_u585_exp22_state.dma_used = 0U;

  U585_Log_WriteLine("");
  U585_Log_WriteLine("[U585][22] ADF1 MIC1 PE9=CCK0 PE10=SDI0");
  U585_Log_WriteLine("[U585][22] mode=poll SINC4 decim=24 dma_used=0");

  g_u585_exp22_state.adf_ready = (U585_ADF1_Init() == HAL_OK) ? 1U : 0U;
  U585_Log_WriteU32("[U585][22] adf_ready=", g_u585_exp22_state.adf_ready);
  U585_Log_WriteU32("[U585][22] dma_used=", g_u585_exp22_state.dma_used);

  if (g_u585_exp22_state.adf_ready != 0U)
  {
    exp22_sample();
  }
}

static void exp22_loop(void)
{
  static uint8_t last_button = 0U;
  uint8_t button = U585_Board_IsUserButtonPressed();

  g_u585_exp22_state.iterations++;
  g_u585_exp22_state.tick_ms = HAL_GetTick();

  if ((button != 0U) && (last_button == 0U) && (g_u585_exp22_state.adf_ready != 0U))
  {
    U585_Log_WriteLine("[U585][22] resample...");
    exp22_sample();
  }
  last_button = button;

  if ((g_u585_exp22_state.iterations % 4U) == 0U)
  {
    U585_Log_WriteU32("[U585][22] heartbeat=", g_u585_exp22_state.iterations);
    if (g_u585_exp22_state.adf_ready != 0U)
    {
      exp22_sample();
    }
  }

  U585_Board_ToggleGreenLed();
  HAL_Delay(500U);
}

const U585_Demo U585_Demo_Exp22 = {
  "22",
  "ADF1 digital MEMS mic PDM poll",
  exp22_init,
  exp22_loop,
};
