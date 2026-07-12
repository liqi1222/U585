#include "u585_board.h"
#include "u585_demo.h"
#include "u585_log.h"

typedef struct
{
  uint32_t magic;
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t icache_ready;
  uint32_t cycles_off;
  uint32_t cycles_on;
  uint32_t hit_off;
  uint32_t miss_off;
  uint32_t hit_on;
  uint32_t miss_on;
  uint32_t faster;
  uint32_t checksum;
} U585_Exp29State;

volatile U585_Exp29State g_u585_exp29_state;

/* Large-ish const blob in Flash so ICACHE has something to cache. */
static const uint8_t s_bench_blob[512] = {
  0x01U, 0x23U, 0x45U, 0x67U, 0x89U, 0xABU, 0xCDU, 0xEFU,
  0x10U, 0x32U, 0x54U, 0x76U, 0x98U, 0xBAU, 0xDCU, 0xFEU,
  /* rest zero-initialized by C — enough span for line fills */
};

static void exp29_dwt_enable(void)
{
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0U;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static uint32_t exp29_cycles(void)
{
  return DWT->CYCCNT;
}

__attribute__((noinline))
static uint32_t exp29_workload(uint32_t loops)
{
  uint32_t sum = 0U;
  uint32_t i;

  for (i = 0U; i < loops; i++)
  {
    sum += s_bench_blob[i & 0x1FFU];
    sum ^= (uint32_t)s_bench_blob[(i * 3U) & 0x1FFU] << (i & 7U);
  }
  return sum;
}

static uint32_t exp29_run_once(uint8_t enable_cache, uint32_t *hit, uint32_t *miss)
{
  uint32_t t0;
  uint32_t t1;
  uint32_t sum;

  (void)HAL_ICACHE_Disable();
  (void)HAL_ICACHE_Invalidate();
  (void)HAL_ICACHE_Monitor_Reset(ICACHE_MONITOR_HIT_MISS);
  (void)HAL_ICACHE_Monitor_Stop(ICACHE_MONITOR_HIT_MISS);

  if (enable_cache != 0U)
  {
    (void)HAL_ICACHE_ConfigAssociativityMode(ICACHE_1WAY);
    (void)HAL_ICACHE_Enable();
  }

  (void)HAL_ICACHE_Monitor_Start(ICACHE_MONITOR_HIT_MISS);
  exp29_dwt_enable();
  t0 = exp29_cycles();
  sum = exp29_workload(20000U);
  t1 = exp29_cycles();
  (void)HAL_ICACHE_Monitor_Stop(ICACHE_MONITOR_HIT_MISS);

  *hit = HAL_ICACHE_Monitor_GetHitValue();
  *miss = HAL_ICACHE_Monitor_GetMissValue();
  g_u585_exp29_state.checksum = sum;
  return (t1 - t0);
}

static void exp29_bench(void)
{
  uint32_t hit = 0U;
  uint32_t miss = 0U;

  g_u585_exp29_state.cycles_off = exp29_run_once(0U, &hit, &miss);
  g_u585_exp29_state.hit_off = hit;
  g_u585_exp29_state.miss_off = miss;

  g_u585_exp29_state.cycles_on = exp29_run_once(1U, &hit, &miss);
  g_u585_exp29_state.hit_on = hit;
  g_u585_exp29_state.miss_on = miss;

  g_u585_exp29_state.faster =
      (g_u585_exp29_state.cycles_on < g_u585_exp29_state.cycles_off) ? 1U : 0U;

  U585_Log_WriteU32("[U585][29] cycles_off=", g_u585_exp29_state.cycles_off);
  U585_Log_WriteU32("[U585][29] cycles_on=", g_u585_exp29_state.cycles_on);
  U585_Log_WriteU32("[U585][29] hit_off=", g_u585_exp29_state.hit_off);
  U585_Log_WriteU32("[U585][29] miss_off=", g_u585_exp29_state.miss_off);
  U585_Log_WriteU32("[U585][29] hit_on=", g_u585_exp29_state.hit_on);
  U585_Log_WriteU32("[U585][29] miss_on=", g_u585_exp29_state.miss_on);
  U585_Log_WriteU32("[U585][29] faster=", g_u585_exp29_state.faster);
  U585_Log_WriteU32("[U585][29] checksum=", g_u585_exp29_state.checksum);
}

static void exp29_init(void)
{
  U585_Board_InitBasicGpio();
  g_u585_exp29_state.magic = 0xA585001DUL;
  g_u585_exp29_state.iterations = 0U;
  g_u585_exp29_state.tick_ms = HAL_GetTick();

  U585_Log_WriteLine("");
  U585_Log_WriteLine("[U585][29] ICACHE on/off fetch bench (DWT CYCCNT)");
  U585_Log_WriteLine("[U585][29] internal Flash workload (OSPI XIP contrast later)");

  /* ICACHE_REG handed to NS; must disable before changing WAYSEL. */
  (void)HAL_ICACHE_Disable();
  g_u585_exp29_state.icache_ready =
      (HAL_ICACHE_ConfigAssociativityMode(ICACHE_1WAY) == HAL_OK) ? 1U : 0U;
  U585_Log_WriteU32("[U585][29] icache_ready=", g_u585_exp29_state.icache_ready);

  if (g_u585_exp29_state.icache_ready != 0U)
  {
    exp29_bench();
    /* Leave ICACHE enabled for the rest of the session. */
    (void)HAL_ICACHE_Enable();
  }
}

static void exp29_loop(void)
{
  static uint8_t last_button = 0U;
  uint8_t button = U585_Board_IsUserButtonPressed();

  g_u585_exp29_state.iterations++;
  g_u585_exp29_state.tick_ms = HAL_GetTick();

  if ((button != 0U) && (last_button == 0U) && (g_u585_exp29_state.icache_ready != 0U))
  {
    U585_Log_WriteLine("[U585][29] rebench...");
    exp29_bench();
    (void)HAL_ICACHE_Enable();
  }
  last_button = button;

  if ((g_u585_exp29_state.iterations % 4U) == 0U)
  {
    U585_Log_WriteU32("[U585][29] heartbeat=", g_u585_exp29_state.iterations);
  }

  U585_Board_ToggleGreenLed();
  HAL_Delay(500U);
}

const U585_Demo U585_Demo_Exp29 = {
  "29",
  "ICACHE on/off DWT bench",
  exp29_init,
  exp29_loop,
};
