#include "u585_board.h"
#include "u585_demo.h"
#include "u585_log.h"

typedef struct
{
  uint32_t magic;
  uint32_t system_core_clock_hz;
  uint32_t sysclk_hz;
  uint32_t hclk_hz;
  uint32_t pclk1_hz;
  uint32_t pclk2_hz;
  uint32_t pclk3_hz;
  uint32_t flash_latency;
  uint32_t capture_count;
} U585_ClockSnapshot;

volatile U585_ClockSnapshot g_u585_clock_snapshot;

static void exp05_capture_clock_snapshot(void)
{
  g_u585_clock_snapshot.magic = 0xA5850005UL;
  g_u585_clock_snapshot.system_core_clock_hz = SystemCoreClock;
  g_u585_clock_snapshot.sysclk_hz = HAL_RCC_GetSysClockFreq();
  g_u585_clock_snapshot.hclk_hz = HAL_RCC_GetHCLKFreq();
  g_u585_clock_snapshot.pclk1_hz = HAL_RCC_GetPCLK1Freq();
  g_u585_clock_snapshot.pclk2_hz = HAL_RCC_GetPCLK2Freq();
  g_u585_clock_snapshot.pclk3_hz = HAL_RCC_GetPCLK3Freq();
  g_u585_clock_snapshot.flash_latency = (FLASH->ACR & FLASH_ACR_LATENCY);
  g_u585_clock_snapshot.capture_count++;
}

static void exp05_log_snapshot(void)
{
  U585_Log_WriteU32("[U585][05] SystemCoreClock_Hz=", g_u585_clock_snapshot.system_core_clock_hz);
  U585_Log_WriteU32("[U585][05] SYSCLK_Hz=", g_u585_clock_snapshot.sysclk_hz);
  U585_Log_WriteU32("[U585][05] HCLK_Hz=", g_u585_clock_snapshot.hclk_hz);
  U585_Log_WriteU32("[U585][05] PCLK1_Hz=", g_u585_clock_snapshot.pclk1_hz);
  U585_Log_WriteU32("[U585][05] PCLK2_Hz=", g_u585_clock_snapshot.pclk2_hz);
  U585_Log_WriteU32("[U585][05] PCLK3_Hz=", g_u585_clock_snapshot.pclk3_hz);
  U585_Log_WriteU32("[U585][05] FLASH_LATENCY=", g_u585_clock_snapshot.flash_latency);
}

static void exp05_init(void)
{
  U585_Board_InitBasicGpio();
  g_u585_clock_snapshot.capture_count = 0U;
  exp05_capture_clock_snapshot();

  U585_Log_WriteLine("");
  U585_Log_WriteLine("[U585][05] clock tree observation start");
  U585_Log_WriteLine("[U585][05] expected: MSI->PLL ~160MHz SCALE1 LATENCY4");
  exp05_log_snapshot();
}

static void exp05_loop(void)
{
  exp05_capture_clock_snapshot();
  if ((g_u585_clock_snapshot.capture_count % 5U) == 0U)
  {
    exp05_log_snapshot();
  }
  U585_Board_ToggleGreenLed();
  HAL_Delay(1000U);
}

const U585_Demo U585_Demo_Exp05ClockTree = {
  "05",
  "Clock tree and PLL baseline observation",
  exp05_init,
  exp05_loop,
};
