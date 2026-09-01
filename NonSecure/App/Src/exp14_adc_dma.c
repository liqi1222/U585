#include "u585_adc1.h"
#include "u585_board.h"
#include "u585_demo.h"
#include "u585_gpdma.h"
#include "u585_log.h"
#include "u585_tim2_trgo.h"

#define U585_EXP14_SAMPLE_COUNT (128U)

typedef struct
{
  uint32_t magic;
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t adc_ready;
  uint32_t timer_ready;
  uint32_t dma_started;
  uint32_t dma_used;
  uint32_t sample_rate_hz;
  uint32_t dma_half_count;
  uint32_t dma_full_count;
  uint32_t vrefint_raw;
  uint32_t vrefint_cal;
  uint32_t vdda_mv;
  uint32_t dma_csr;
} U585_Exp14State;

volatile U585_Exp14State g_u585_exp14_state;
static __ALIGNED(32) uint16_t s_exp14_samples[U585_EXP14_SAMPLE_COUNT];

static uint16_t exp14_read_vrefint_cal(void)
{
  /* VREFINT_CAL lives at an odd address; byte reads avoid alignment traps. */
  const uint8_t *p = (const uint8_t *)VREFINT_CAL_ADDR;
  return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static void exp14_update_vdda(void)
{
  uint32_t vref_raw = g_u585_exp14_state.vrefint_raw;
  uint32_t vref_cal = g_u585_exp14_state.vrefint_cal;

  /* VREFINT_CAL is factory-trimmed at ADC1's 14-bit resolution. */
  if ((vref_raw != 0U) && (vref_cal != 0U) && (vref_cal <= 0x3FFFU))
  {
    g_u585_exp14_state.vdda_mv = (VREFINT_CAL_VREF * vref_cal) / (vref_raw << 2U);
  }
  else if (vref_raw != 0U)
  {
    /* Fallback only when the factory calibration word is not readable/valid. */
    g_u585_exp14_state.vdda_mv = (1212UL * 4095UL) / vref_raw;
  }
  else
  {
    g_u585_exp14_state.vdda_mv = 0U;
  }
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc)
{
  if (hadc == &hadc1_ns)
  {
    g_u585_exp14_state.dma_half_count++;
    g_u585_exp14_state.vrefint_raw = s_exp14_samples[(U585_EXP14_SAMPLE_COUNT / 2U) - 1U];
  }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
  if (hadc == &hadc1_ns)
  {
    g_u585_exp14_state.dma_full_count++;
    g_u585_exp14_state.vrefint_raw = s_exp14_samples[U585_EXP14_SAMPLE_COUNT - 1U];
  }
}

static void exp14_report(void)
{
  exp14_update_vdda();
  g_u585_exp14_state.dma_csr = hdma_gpdma1_ch1_adc_ns.Instance->CSR;
  U585_Log_WriteU32("[U585][14] vrefint_raw=", g_u585_exp14_state.vrefint_raw);
  U585_Log_WriteU32("[U585][14] vdda_mv=", g_u585_exp14_state.vdda_mv);
  U585_Log_WriteU32("[U585][14] dma_half_count=", g_u585_exp14_state.dma_half_count);
  U585_Log_WriteU32("[U585][14] dma_full_count=", g_u585_exp14_state.dma_full_count);
  U585_Log_WriteU32("[U585][14] dma_csr=", g_u585_exp14_state.dma_csr);
}

static void exp14_init(void)
{
  U585_Board_InitBasicGpio();
  g_u585_exp14_state.magic = 0xA585000EUL;
  g_u585_exp14_state.iterations = 0U;
  g_u585_exp14_state.tick_ms = HAL_GetTick();
  g_u585_exp14_state.dma_used = 1U;
  g_u585_exp14_state.sample_rate_hz = U585_TIM2_TRGO_GetHz();
  g_u585_exp14_state.vrefint_cal = exp14_read_vrefint_cal();

  g_u585_exp14_state.adc_ready = (U585_ADC1_InitTimerDma() == HAL_OK) ? 1U : 0U;
  if (g_u585_exp14_state.adc_ready != 0U)
  {
    g_u585_exp14_state.timer_ready = (U585_TIM2_TRGO_Init() == HAL_OK) ? 1U : 0U;
  }
  if ((g_u585_exp14_state.adc_ready != 0U) && (g_u585_exp14_state.timer_ready != 0U) &&
      (U585_ADC1_StartTimerDma(s_exp14_samples, U585_EXP14_SAMPLE_COUNT) == HAL_OK) &&
      (U585_TIM2_TRGO_Start() == HAL_OK))
  {
    g_u585_exp14_state.dma_started = 1U;
  }

  U585_Log_WriteLine("");
  U585_Log_WriteLine("[U585][14] ADC1 VREFINT: TIM2 TRGO + GPDMA circular list");
  U585_Log_WriteU32("[U585][14] adc_ready=", g_u585_exp14_state.adc_ready);
  U585_Log_WriteU32("[U585][14] timer_ready=", g_u585_exp14_state.timer_ready);
  U585_Log_WriteU32("[U585][14] dma_used=", g_u585_exp14_state.dma_used);
  U585_Log_WriteU32("[U585][14] dma_started=", g_u585_exp14_state.dma_started);
  U585_Log_WriteU32("[U585][14] sample_rate_hz=", g_u585_exp14_state.sample_rate_hz);
  U585_Log_WriteU32("[U585][14] vrefint_cal=", g_u585_exp14_state.vrefint_cal);
}

static void exp14_loop(void)
{
  g_u585_exp14_state.iterations++;
  g_u585_exp14_state.tick_ms = HAL_GetTick();
  exp14_report();
  U585_Board_ToggleGreenLed();
  HAL_Delay(500U);
}

const U585_Demo U585_Demo_Exp14 = {
  "14",
  "ADC1 VREFINT timer-paced GPDMA stream",
  exp14_init,
  exp14_loop,
};
