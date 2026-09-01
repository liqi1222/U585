#include "u585_board.h"
#include "u585_demo.h"
#include "u585_log.h"
#include "u585_ospi.h"

typedef struct
{
  uint32_t magic;
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t ospi_psram_ready;
  uint32_t memtest_ok;
  uint32_t wrote;
  uint32_t readback;
} U585_Exp24State;

volatile U585_Exp24State g_u585_exp24_state;

static void exp24_memtest(void)
{
  uint32_t wrote = 0U;
  uint32_t readback = 0U;

  g_u585_exp24_state.memtest_ok = 0U;
  g_u585_exp24_state.wrote = 0U;
  g_u585_exp24_state.readback = 0U;
  if (U585_OSPI_Psram_MemTest(&wrote, &readback) == HAL_OK)
  {
    g_u585_exp24_state.memtest_ok = 1U;
  }
  g_u585_exp24_state.wrote = wrote;
  g_u585_exp24_state.readback = readback;
  U585_Log_WriteU32("[U585][24] memtest_ok=", g_u585_exp24_state.memtest_ok);
  U585_Log_WriteU32("[U585][24] id_or_wrote=", g_u585_exp24_state.wrote);
  U585_Log_WriteU32("[U585][24] readback=", g_u585_exp24_state.readback);
}

static void exp24_init(void)
{
  U585_Board_InitBasicGpio();
  g_u585_exp24_state.magic = 0xA5850018UL;
  g_u585_exp24_state.iterations = 0U;
  g_u585_exp24_state.tick_ms = HAL_GetTick();

  U585_Log_WriteLine("");
  U585_Log_WriteLine("[U585][24] OCTOSPI1 APS6408 PSRAM @0x90000000");
  U585_Log_WriteLine("[U585][24] mode=SPI write/read (octal/MM pending)");

  g_u585_exp24_state.ospi_psram_ready = (U585_OSPI_Psram_Init() == HAL_OK) ? 1U : 0U;
  U585_Log_WriteU32("[U585][24] ospi_psram_ready=", g_u585_exp24_state.ospi_psram_ready);

  if (g_u585_exp24_state.ospi_psram_ready != 0U)
  {
    exp24_memtest();
  }
}

static void exp24_loop(void)
{
  static uint8_t last_button = 0U;
  uint8_t button = U585_Board_IsUserButtonPressed();

  g_u585_exp24_state.iterations++;
  g_u585_exp24_state.tick_ms = HAL_GetTick();

  if ((button != 0U) && (last_button == 0U) && (g_u585_exp24_state.ospi_psram_ready != 0U))
  {
    U585_Log_WriteLine("[U585][24] re-run memtest...");
    exp24_memtest();
  }
  last_button = button;

  if ((g_u585_exp24_state.iterations % 4U) == 0U)
  {
    U585_Log_WriteU32("[U585][24] heartbeat=", g_u585_exp24_state.iterations);
  }

  U585_Board_ToggleGreenLed();
  HAL_Delay(500U);
}

const U585_Demo U585_Demo_Exp24 = {
  "24",
  "OCTOSPI1 PSRAM memtest",
  exp24_init,
  exp24_loop,
};
