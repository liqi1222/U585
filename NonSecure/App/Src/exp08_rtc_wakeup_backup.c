#include "u585_board.h"
#include "u585_demo.h"
#include "u585_log.h"

RTC_HandleTypeDef hrtc_ns;

typedef struct
{
  uint32_t magic;
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t rtc_ready;
  uint32_t hours;
  uint32_t minutes;
  uint32_t seconds;
  uint32_t backup0;
} U585_Exp08State;

volatile U585_Exp08State g_u585_exp08_state;

static HAL_StatusTypeDef exp08_rtc_init(void)
{
  RCC_OscInitTypeDef osc = {0};
  RCC_PeriphCLKInitTypeDef periph = {0};
  RTC_TimeTypeDef time = {0};
  RTC_DateTypeDef date = {0};

  HAL_PWR_EnableBkUpAccess();

  osc.OscillatorType = RCC_OSCILLATORTYPE_LSI;
  osc.LSIState = RCC_LSI_ON;
  if (HAL_RCC_OscConfig(&osc) != HAL_OK)
  {
    return HAL_ERROR;
  }

  periph.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  periph.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&periph) != HAL_OK)
  {
    return HAL_ERROR;
  }

  __HAL_RCC_RTC_ENABLE();
  __HAL_RCC_RTCAPB_CLK_ENABLE();

  hrtc_ns.Instance = RTC;
  hrtc_ns.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc_ns.Init.AsynchPrediv = 127;
  hrtc_ns.Init.SynchPrediv = 249;
  hrtc_ns.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc_ns.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc_ns.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc_ns.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  hrtc_ns.Init.OutPutPullUp = RTC_OUTPUT_PULLUP_NONE;
  hrtc_ns.Init.BinMode = RTC_BINARY_NONE;
  if (HAL_RTC_Init(&hrtc_ns) != HAL_OK)
  {
    return HAL_ERROR;
  }

  time.Hours = 12;
  time.Minutes = 0;
  time.Seconds = 0;
  time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  time.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc_ns, &time, RTC_FORMAT_BIN) != HAL_OK)
  {
    return HAL_ERROR;
  }

  date.WeekDay = RTC_WEEKDAY_SUNDAY;
  date.Month = RTC_MONTH_JULY;
  date.Date = 12;
  date.Year = 26;
  if (HAL_RTC_SetDate(&hrtc_ns, &date, RTC_FORMAT_BIN) != HAL_OK)
  {
    return HAL_ERROR;
  }

  HAL_RTCEx_BKUPWrite(&hrtc_ns, RTC_BKP_DR0, 0xA5850008UL);
  return HAL_OK;
}

static void exp08_init(void)
{
  U585_Board_InitBasicGpio();
  g_u585_exp08_state.magic = 0xA5850008UL;
  g_u585_exp08_state.iterations = 0U;
  g_u585_exp08_state.tick_ms = HAL_GetTick();
  g_u585_exp08_state.rtc_ready = (exp08_rtc_init() == HAL_OK) ? 1U : 0U;
  g_u585_exp08_state.backup0 = HAL_RTCEx_BKUPRead(&hrtc_ns, RTC_BKP_DR0);

  U585_Log_WriteLine("");
  U585_Log_WriteLine("[U585][08] RTC NonSecure calendar/backup demo");
  U585_Log_WriteU32("[U585][08] rtc_ready=", g_u585_exp08_state.rtc_ready);
  U585_Log_WriteU32("[U585][08] backup0=", g_u585_exp08_state.backup0);
  U585_Log_WriteLine("[U585][08] NOTE: Stop/Standby wakeup current -> author DMM");
}

static void exp08_loop(void)
{
  RTC_TimeTypeDef time = {0};
  RTC_DateTypeDef date = {0};

  g_u585_exp08_state.iterations++;
  g_u585_exp08_state.tick_ms = HAL_GetTick();

  if (g_u585_exp08_state.rtc_ready != 0U)
  {
    (void)HAL_RTC_GetTime(&hrtc_ns, &time, RTC_FORMAT_BIN);
    (void)HAL_RTC_GetDate(&hrtc_ns, &date, RTC_FORMAT_BIN);
    g_u585_exp08_state.hours = time.Hours;
    g_u585_exp08_state.minutes = time.Minutes;
    g_u585_exp08_state.seconds = time.Seconds;
    U585_Log_WriteU32("[U585][08] seconds=", g_u585_exp08_state.seconds);
  }

  U585_Board_ToggleGreenLed();
  HAL_Delay(1000U);
}

const U585_Demo U585_Demo_Exp08 = {
  "08",
  "RTC wakeup and backup-domain experiment",
  exp08_init,
  exp08_loop,
};
