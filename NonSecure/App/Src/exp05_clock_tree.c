#include "u585_board.h"
#include "u585_demo.h"

typedef struct
{
  uint32_t system_core_clock_hz;
  uint32_t sysclk_hz;
  uint32_t hclk_hz;
  uint32_t pclk1_hz;
  uint32_t pclk2_hz;
  uint32_t pclk3_hz;
} U585_ClockSnapshot;

volatile U585_ClockSnapshot g_u585_clock_snapshot;

static void exp05_capture_clock_snapshot(void)
{
  g_u585_clock_snapshot.system_core_clock_hz = SystemCoreClock;
  g_u585_clock_snapshot.sysclk_hz = HAL_RCC_GetSysClockFreq();
  g_u585_clock_snapshot.hclk_hz = HAL_RCC_GetHCLKFreq();
  g_u585_clock_snapshot.pclk1_hz = HAL_RCC_GetPCLK1Freq();
  g_u585_clock_snapshot.pclk2_hz = HAL_RCC_GetPCLK2Freq();
  g_u585_clock_snapshot.pclk3_hz = HAL_RCC_GetPCLK3Freq();
}

static void exp05_init(void)
{
  U585_Board_InitBasicGpio();
  exp05_capture_clock_snapshot();
}

static void exp05_loop(void)
{
  exp05_capture_clock_snapshot();
  U585_Board_ToggleGreenLed();
  HAL_Delay(1000U);
}

const U585_Demo U585_Demo_Exp05ClockTree = {
  "05",
  "Clock tree and PLL baseline observation",
  exp05_init,
  exp05_loop,
};
