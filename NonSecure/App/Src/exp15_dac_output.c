#include "u585_board.h"
#include "u585_dac1.h"
#include "u585_demo.h"
#include "u585_log.h"

typedef struct
{
  uint32_t magic;
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t dac_ready;
  uint32_t code;
  uint32_t dor;
  uint32_t dma_used;
} U585_Exp15State;

volatile U585_Exp15State g_u585_exp15_state;

static const uint32_t s_codes[3] = {0U, 2048U, 4095U};

static void exp15_apply(uint32_t code)
{
  g_u585_exp15_state.code = code;
  g_u585_exp15_state.dor = 0U;
  if (U585_DAC1_SetCode12(code) == HAL_OK)
  {
    /* Allow analog settle before reading DOR. */
    HAL_Delay(1U);
    g_u585_exp15_state.dor = U585_DAC1_GetDor();
  }
  U585_Log_WriteU32("[U585][15] code=", g_u585_exp15_state.code);
  U585_Log_WriteU32("[U585][15] dor=", g_u585_exp15_state.dor);
}

static void exp15_init(void)
{
  U585_Board_InitBasicGpio();
  g_u585_exp15_state.magic = 0xA585000FUL;
  g_u585_exp15_state.iterations = 0U;
  g_u585_exp15_state.tick_ms = HAL_GetTick();
  g_u585_exp15_state.dma_used = 0U;

  U585_Log_WriteLine("");
  U585_Log_WriteLine("[U585][15] DAC1_OUT1 on PA4 (STMOD NSS shared)");
  U585_Log_WriteLine("[U585][15] mode=DC codes dma_used=0");

  g_u585_exp15_state.dac_ready = (U585_DAC1_Init() == HAL_OK) ? 1U : 0U;
  U585_Log_WriteU32("[U585][15] dac_ready=", g_u585_exp15_state.dac_ready);
  U585_Log_WriteU32("[U585][15] dma_used=", g_u585_exp15_state.dma_used);

  if (g_u585_exp15_state.dac_ready != 0U)
  {
    exp15_apply(2048U);
  }
}

static void exp15_loop(void)
{
  static uint8_t last_button = 0U;
  static uint32_t code_idx = 1U;
  uint8_t button = U585_Board_IsUserButtonPressed();

  g_u585_exp15_state.iterations++;
  g_u585_exp15_state.tick_ms = HAL_GetTick();

  if ((button != 0U) && (last_button == 0U) && (g_u585_exp15_state.dac_ready != 0U))
  {
    code_idx = (code_idx + 1U) % 3U;
    U585_Log_WriteLine("[U585][15] next code...");
    exp15_apply(s_codes[code_idx]);
  }
  last_button = button;

  if ((g_u585_exp15_state.iterations % 4U) == 0U)
  {
    U585_Log_WriteU32("[U585][15] heartbeat=", g_u585_exp15_state.iterations);
    U585_Log_WriteU32("[U585][15] dor=", U585_DAC1_GetDor());
  }

  U585_Board_ToggleGreenLed();
  HAL_Delay(500U);
}

const U585_Demo U585_Demo_Exp15 = {
  "15",
  "DAC1 DC output on PA4",
  exp15_init,
  exp15_loop,
};
