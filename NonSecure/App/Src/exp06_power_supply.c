#include "u585_board.h"
#include "u585_demo.h"
#include "u585_log.h"

typedef struct
{
  uint32_t magic;
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t voltage_range;
  uint32_t vosr_raw;
  uint32_t cr3_raw;
  uint32_t smps_selected;
  uint32_t requires_power_measurement;
} U585_Exp06State;

volatile U585_Exp06State g_u585_exp06_state;

static void exp06_capture_power(void)
{
  g_u585_exp06_state.magic = 0xA5850006UL;
  g_u585_exp06_state.tick_ms = HAL_GetTick();
  g_u585_exp06_state.voltage_range = HAL_PWREx_GetVoltageRange();
  g_u585_exp06_state.vosr_raw = PWR->VOSR;
  g_u585_exp06_state.cr3_raw = PWR->CR3;
  g_u585_exp06_state.smps_selected = (READ_BIT(PWR->CR3, PWR_CR3_REGSEL) != 0U) ? 1U : 0U;
  g_u585_exp06_state.requires_power_measurement = 1U;
}

static void exp06_log_power(void)
{
  U585_Log_WriteU32("[U585][06] voltage_range=", g_u585_exp06_state.voltage_range);
  U585_Log_WriteU32("[U585][06] PWR_VOSR=", g_u585_exp06_state.vosr_raw);
  U585_Log_WriteU32("[U585][06] PWR_CR3=", g_u585_exp06_state.cr3_raw);
  U585_Log_WriteU32("[U585][06] smps_selected=", g_u585_exp06_state.smps_selected);
  U585_Log_WriteLine("[U585][06] NOTE: board current needs DMM on JP measurement path");
}

static void exp06_init(void)
{
  U585_Board_InitBasicGpio();
  g_u585_exp06_state.iterations = 0U;
  exp06_capture_power();

  U585_Log_WriteLine("");
  U585_Log_WriteLine("[U585][06] power architecture observation start");
  U585_Log_WriteLine("[U585][06] Secure SystemPower_Config: SMPS + VOS1");
  exp06_log_power();
}

static void exp06_loop(void)
{
  g_u585_exp06_state.iterations++;
  exp06_capture_power();
  if ((g_u585_exp06_state.iterations % 5U) == 0U)
  {
    exp06_log_power();
  }
  U585_Board_ToggleGreenLed();
  HAL_Delay(1000U);
}

const U585_Demo U585_Demo_Exp06 = {
  "06",
  "Power architecture: SMPS/LDO and voltage scale",
  exp06_init,
  exp06_loop,
};
