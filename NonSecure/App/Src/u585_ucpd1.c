#include "u585_ucpd1.h"
#include "stm32u5xx_ll_ucpd.h"
#include "stm32u5xx_ll_bus.h"

static uint8_t u585_ucpd1_ready = 0U;

HAL_StatusTypeDef U585_UCPD1_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  u585_ucpd1_ready = 0U;

  LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_UCPD1);
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* PA15=CC1, PB15=CC2 — analog for Type-C CC sensing. */
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  LL_UCPD_Disable(UCPD1);

  /* Manual CFG1 (avoid linking LL_UCPD_Init / USE_FULL_LL_DRIVER). */
  WRITE_REG(UCPD1->CFG1,
            LL_UCPD_PSC_DIV2 |
            (0x07UL << UCPD_CFG1_TRANSWIN_Pos) |
            (0x10UL << UCPD_CFG1_IFRGAP_Pos) |
            0x0DUL);

  LL_UCPD_SetSNKRole(UCPD1);
  LL_UCPD_SetRpResistor(UCPD1, LL_UCPD_RESISTOR_NONE);
  LL_UCPD_SetccEnable(UCPD1, LL_UCPD_CCENABLE_CC1CC2);
  LL_UCPD_Enable(UCPD1);

  /* Allow Type-C detector to settle. */
  HAL_Delay(5U);

  u585_ucpd1_ready = 1U;
  return HAL_OK;
}

uint8_t U585_UCPD1_IsReady(void)
{
  return u585_ucpd1_ready;
}

uint32_t U585_UCPD1_GetCc1State(void)
{
  return LL_UCPD_GetTypeCVstateCC1(UCPD1);
}

uint32_t U585_UCPD1_GetCc2State(void)
{
  return LL_UCPD_GetTypeCVstateCC2(UCPD1);
}

uint32_t U585_UCPD1_GetSr(void)
{
  return LL_UCPD_ReadReg(UCPD1, SR);
}
