#include "u585_board.h"
#include "u585_demo.h"
#include "u585_gpdma.h"
#include "u585_log.h"

#define U585_EXP16_WORDS 16U

typedef struct
{
  uint32_t magic;
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t dma_ready;
  uint32_t xfer_ok;
  uint32_t mismatch;
  uint32_t src0;
  uint32_t dst0;
} U585_Exp16State;

volatile U585_Exp16State g_u585_exp16_state;

static volatile uint32_t s_src[U585_EXP16_WORDS];
static volatile uint32_t s_dst[U585_EXP16_WORDS];

static void exp16_run_copy(void)
{
  uint32_t i;
  uint32_t mismatch = 0U;

  for (i = 0U; i < U585_EXP16_WORDS; i++)
  {
    s_src[i] = 0xA5000000UL + i;
    s_dst[i] = 0U;
  }

  g_u585_exp16_state.xfer_ok = 0U;
  g_u585_exp16_state.mismatch = 0xFFFFFFFFUL;
  g_u585_exp16_state.src0 = s_src[0];
  g_u585_exp16_state.dst0 = 0U;

  if (U585_GPDMA_MemCopyWords((const uint32_t *)&s_src[0], (uint32_t *)&s_dst[0], U585_EXP16_WORDS) == HAL_OK)
  {
    g_u585_exp16_state.xfer_ok = 1U;
    for (i = 0U; i < U585_EXP16_WORDS; i++)
    {
      if (s_dst[i] != s_src[i])
      {
        mismatch++;
      }
    }
    g_u585_exp16_state.mismatch = mismatch;
    g_u585_exp16_state.dst0 = s_dst[0];
  }
  else
  {
    g_u585_exp16_state.dst0 = s_dst[0];
    g_u585_exp16_state.mismatch = 0xEEEEUL;
  }

  U585_Log_WriteU32("[U585][16] xfer_ok=", g_u585_exp16_state.xfer_ok);
  U585_Log_WriteU32("[U585][16] words=", U585_EXP16_WORDS);
  U585_Log_WriteU32("[U585][16] mismatch=", g_u585_exp16_state.mismatch);
  U585_Log_WriteU32("[U585][16] src0=", g_u585_exp16_state.src0);
  U585_Log_WriteU32("[U585][16] dst0=", g_u585_exp16_state.dst0);
}

static void exp16_init(void)
{
  U585_Board_InitBasicGpio();
  g_u585_exp16_state.magic = 0xA5850010UL;
  g_u585_exp16_state.iterations = 0U;
  g_u585_exp16_state.tick_ms = HAL_GetTick();

  U585_Log_WriteLine("");
  U585_Log_WriteLine("[U585][16] GPDMA1_CH0 memory-to-memory");

  g_u585_exp16_state.dma_ready = (U585_GPDMA_Init() == HAL_OK) ? 1U : 0U;
  U585_Log_WriteU32("[U585][16] dma_ready=", g_u585_exp16_state.dma_ready);

  if (g_u585_exp16_state.dma_ready != 0U)
  {
    exp16_run_copy();
  }
}

static void exp16_loop(void)
{
  static uint8_t last_button = 0U;
  uint8_t button = U585_Board_IsUserButtonPressed();

  g_u585_exp16_state.iterations++;
  g_u585_exp16_state.tick_ms = HAL_GetTick();

  if ((button != 0U) && (last_button == 0U) && (g_u585_exp16_state.dma_ready != 0U))
  {
    U585_Log_WriteLine("[U585][16] recopy...");
    exp16_run_copy();
  }
  last_button = button;

  if ((g_u585_exp16_state.iterations % 4U) == 0U)
  {
    U585_Log_WriteU32("[U585][16] heartbeat=", g_u585_exp16_state.iterations);
  }

  U585_Board_ToggleGreenLed();
  HAL_Delay(500U);
}

const U585_Demo U585_Demo_Exp16 = {
  "16",
  "GPDMA1 CH0 mem-to-mem copy",
  exp16_init,
  exp16_loop,
};
