#include "u585_adc1.h"
#include "u585_board.h"
#include "u585_demo.h"
#include "u585_log.h"

typedef struct
{
  uint32_t magic;
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t adc_ready;
  uint32_t resolution_bits;
  uint32_t vrefint_raw;
  uint32_t vrefint_cal;
  uint32_t vdda_mv;
  uint32_t tempsensor_raw;
  uint32_t dma_used;
} U585_Exp14State;

volatile U585_Exp14State g_u585_exp14_state;

static uint16_t exp14_read_vrefint_cal(void)
{
  /* VREFINT_CAL lives at an odd address; read bytes to avoid alignment traps. */
  const uint8_t *p = (const uint8_t *)VREFINT_CAL_ADDR;
  return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static void exp14_sample(void)
{
  uint32_t vref_raw = 0U;
  uint32_t temp_raw = 0U;
  uint16_t vref_cal = exp14_read_vrefint_cal();

  g_u585_exp14_state.vrefint_cal = vref_cal;
  g_u585_exp14_state.vrefint_raw = 0U;
  g_u585_exp14_state.vdda_mv = 0U;
  g_u585_exp14_state.tempsensor_raw = 0U;

  if (U585_ADC1_ReadChannel(ADC_CHANNEL_VREFINT, &vref_raw) == HAL_OK)
  {
    g_u585_exp14_state.vrefint_raw = vref_raw;
    if ((vref_raw != 0U) && (vref_cal >= 1000U) && (vref_cal <= 2000U))
    {
      g_u585_exp14_state.vdda_mv = (VREFINT_CAL_VREF * (uint32_t)vref_cal) / vref_raw;
    }
    else if (vref_raw != 0U)
    {
      /* Fallback when factory cal is unreadable from NS (TrustZone/OTP). */
      g_u585_exp14_state.vdda_mv = (1212UL * 4095UL) / vref_raw;
    }
  }

  if (U585_ADC1_ReadChannel(ADC_CHANNEL_TEMPSENSOR, &temp_raw) == HAL_OK)
  {
    g_u585_exp14_state.tempsensor_raw = temp_raw;
  }

  U585_Log_WriteU32("[U585][14] vrefint_raw=", g_u585_exp14_state.vrefint_raw);
  U585_Log_WriteU32("[U585][14] vrefint_cal=", g_u585_exp14_state.vrefint_cal);
  U585_Log_WriteU32("[U585][14] vdda_mv=", g_u585_exp14_state.vdda_mv);
  U585_Log_WriteU32("[U585][14] tempsensor_raw=", g_u585_exp14_state.tempsensor_raw);
}

static void exp14_init(void)
{
  U585_Board_InitBasicGpio();
  g_u585_exp14_state.magic = 0xA585000EUL;
  g_u585_exp14_state.iterations = 0U;
  g_u585_exp14_state.tick_ms = HAL_GetTick();
  g_u585_exp14_state.resolution_bits = 12U;
  g_u585_exp14_state.dma_used = 0U; /* polling first; GPDMA continuous in demo 16 */

  U585_Log_WriteLine("");
  U585_Log_WriteLine("[U585][14] ADC1 internal channels (VREFINT/TEMP)");
  U585_Log_WriteLine("[U585][14] mode=polling dma_used=0");

  g_u585_exp14_state.adc_ready = (U585_ADC1_Init() == HAL_OK) ? 1U : 0U;
  U585_Log_WriteU32("[U585][14] adc_ready=", g_u585_exp14_state.adc_ready);
  U585_Log_WriteU32("[U585][14] resolution_bits=", g_u585_exp14_state.resolution_bits);
  U585_Log_WriteU32("[U585][14] dma_used=", g_u585_exp14_state.dma_used);

  if (g_u585_exp14_state.adc_ready != 0U)
  {
    exp14_sample();
  }
}

static void exp14_loop(void)
{
  g_u585_exp14_state.iterations++;
  g_u585_exp14_state.tick_ms = HAL_GetTick();

  if ((g_u585_exp14_state.adc_ready != 0U) && ((g_u585_exp14_state.iterations % 2U) == 0U))
  {
    exp14_sample();
  }

  U585_Board_ToggleGreenLed();
  HAL_Delay(500U);
}

const U585_Demo U585_Demo_Exp14 = {
  "14",
  "ADC1 VREFINT/TEMP polling (DMA later)",
  exp14_init,
  exp14_loop,
};
