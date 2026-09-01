#include "u585_board.h"
#include "u585_dac1.h"
#include "u585_demo.h"
#include "u585_gpdma.h"
#include "u585_log.h"
#include "u585_tim2_trgo.h"

#define U585_EXP15_WAVE_SAMPLES (128U)

typedef struct
{
  uint32_t magic;
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t dac_ready;
  uint32_t timer_ready;
  uint32_t dma_started;
  uint32_t dma_used;
  uint32_t sample_rate_hz;
  uint32_t waveform_hz;
  uint32_t dma_half_count;
  uint32_t dma_full_count;
  uint32_t dor;
  uint32_t dma_csr;
} U585_Exp15State;

volatile U585_Exp15State g_u585_exp15_state;
static __ALIGNED(32) uint32_t s_exp15_wave[U585_EXP15_WAVE_SAMPLES];

static void exp15_build_triangle(void)
{
  uint32_t index;

  for (index = 0U; index < (U585_EXP15_WAVE_SAMPLES / 2U); index++)
  {
    s_exp15_wave[index] = (4095U * index) / ((U585_EXP15_WAVE_SAMPLES / 2U) - 1U);
  }
  for (index = (U585_EXP15_WAVE_SAMPLES / 2U); index < U585_EXP15_WAVE_SAMPLES; index++)
  {
    s_exp15_wave[index] = (4095U * (U585_EXP15_WAVE_SAMPLES - 1U - index)) /
                          ((U585_EXP15_WAVE_SAMPLES / 2U) - 1U);
  }
}

void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef *hdac)
{
  if (hdac == &hdac1_ns)
  {
    g_u585_exp15_state.dma_half_count++;
  }
}

void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef *hdac)
{
  if (hdac == &hdac1_ns)
  {
    g_u585_exp15_state.dma_full_count++;
  }
}

static void exp15_report(void)
{
  g_u585_exp15_state.dor = U585_DAC1_GetDor();
  g_u585_exp15_state.dma_csr = hdma_gpdma1_ch2_dac_ns.Instance->CSR;
  U585_Log_WriteU32("[U585][15] dor=", g_u585_exp15_state.dor);
  U585_Log_WriteU32("[U585][15] dma_half_count=", g_u585_exp15_state.dma_half_count);
  U585_Log_WriteU32("[U585][15] dma_full_count=", g_u585_exp15_state.dma_full_count);
  U585_Log_WriteU32("[U585][15] dma_csr=", g_u585_exp15_state.dma_csr);
}

static void exp15_init(void)
{
  U585_Board_InitBasicGpio();
  exp15_build_triangle();
  g_u585_exp15_state.magic = 0xA585000FUL;
  g_u585_exp15_state.iterations = 0U;
  g_u585_exp15_state.tick_ms = HAL_GetTick();
  g_u585_exp15_state.dma_used = 1U;
  g_u585_exp15_state.sample_rate_hz = U585_TIM2_TRGO_GetHz();
  g_u585_exp15_state.waveform_hz = U585_TIM2_TRGO_GetHz() / U585_EXP15_WAVE_SAMPLES;

  g_u585_exp15_state.dac_ready = (U585_DAC1_InitTimerDma() == HAL_OK) ? 1U : 0U;
  if (g_u585_exp15_state.dac_ready != 0U)
  {
    g_u585_exp15_state.timer_ready = (U585_TIM2_TRGO_Init() == HAL_OK) ? 1U : 0U;
  }
  if ((g_u585_exp15_state.dac_ready != 0U) && (g_u585_exp15_state.timer_ready != 0U) &&
      (U585_DAC1_StartTimerDma(s_exp15_wave, U585_EXP15_WAVE_SAMPLES) == HAL_OK) &&
      (U585_TIM2_TRGO_Start() == HAL_OK))
  {
    g_u585_exp15_state.dma_started = 1U;
  }

  U585_Log_WriteLine("");
  U585_Log_WriteLine("[U585][15] DAC1_OUT1 PA4: TIM2 TRGO + GPDMA circular list");
  U585_Log_WriteLine("[U585][15] WARNING: PA4 shares STMod+ SPI1_NSS; leave that bus unused.");
  U585_Log_WriteU32("[U585][15] dac_ready=", g_u585_exp15_state.dac_ready);
  U585_Log_WriteU32("[U585][15] timer_ready=", g_u585_exp15_state.timer_ready);
  U585_Log_WriteU32("[U585][15] dma_used=", g_u585_exp15_state.dma_used);
  U585_Log_WriteU32("[U585][15] dma_started=", g_u585_exp15_state.dma_started);
  U585_Log_WriteU32("[U585][15] sample_rate_hz=", g_u585_exp15_state.sample_rate_hz);
  U585_Log_WriteU32("[U585][15] waveform_hz=", g_u585_exp15_state.waveform_hz);
}

static void exp15_loop(void)
{
  g_u585_exp15_state.iterations++;
  g_u585_exp15_state.tick_ms = HAL_GetTick();
  exp15_report();
  U585_Board_ToggleGreenLed();
  HAL_Delay(500U);
}

const U585_Demo U585_Demo_Exp15 = {
  "15",
  "DAC1 triangle timer-paced GPDMA stream",
  exp15_init,
  exp15_loop,
};
