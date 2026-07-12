#include "u585_board.h"
#include "u585_demo.h"
#include "u585_log.h"
#include "u585_ospi.h"

typedef struct
{
  uint32_t magic;
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t ospi_flash_ready;
  uint32_t jedec_ok;
  uint32_t id0;
  uint32_t id1;
  uint32_t id2;
} U585_Exp23State;

volatile U585_Exp23State g_u585_exp23_state;

static void exp23_read_id(void)
{
  uint8_t id[3] = {0};

  g_u585_exp23_state.jedec_ok = 0U;
  g_u585_exp23_state.id0 = 0U;
  g_u585_exp23_state.id1 = 0U;
  g_u585_exp23_state.id2 = 0U;
  if (U585_OSPI_Flash_ReadJedecId(id) == HAL_OK)
  {
    g_u585_exp23_state.jedec_ok = 1U;
    g_u585_exp23_state.id0 = id[0];
    g_u585_exp23_state.id1 = id[1];
    g_u585_exp23_state.id2 = id[2];
  }
  U585_Log_WriteU32("[U585][23] jedec_ok=", g_u585_exp23_state.jedec_ok);
  U585_Log_WriteU32("[U585][23] id0=", g_u585_exp23_state.id0);
  U585_Log_WriteU32("[U585][23] id1=", g_u585_exp23_state.id1);
  U585_Log_WriteU32("[U585][23] id2=", g_u585_exp23_state.id2);
}

static void exp23_init(void)
{
  U585_Board_InitBasicGpio();
  g_u585_exp23_state.magic = 0xA5850017UL;
  g_u585_exp23_state.iterations = 0U;
  g_u585_exp23_state.tick_ms = HAL_GetTick();

  U585_Log_WriteLine("");
  U585_Log_WriteLine("[U585][23] OCTOSPI2 MX25LM51245G JEDEC 0x9F");
  U585_Log_WriteLine("[U585][23] mode=SPI 1-1-1 (XIP pending)");

  g_u585_exp23_state.ospi_flash_ready = (U585_OSPI_Flash_Init() == HAL_OK) ? 1U : 0U;
  U585_Log_WriteU32("[U585][23] ospi_flash_ready=", g_u585_exp23_state.ospi_flash_ready);

  if (g_u585_exp23_state.ospi_flash_ready != 0U)
  {
    exp23_read_id();
  }
}

static void exp23_loop(void)
{
  static uint8_t last_button = 0U;
  uint8_t button = U585_Board_IsUserButtonPressed();

  g_u585_exp23_state.iterations++;
  g_u585_exp23_state.tick_ms = HAL_GetTick();

  if ((button != 0U) && (last_button == 0U) && (g_u585_exp23_state.ospi_flash_ready != 0U))
  {
    U585_Log_WriteLine("[U585][23] re-read JEDEC...");
    exp23_read_id();
  }
  last_button = button;

  if ((g_u585_exp23_state.iterations % 4U) == 0U)
  {
    U585_Log_WriteU32("[U585][23] heartbeat=", g_u585_exp23_state.iterations);
  }

  U585_Board_ToggleGreenLed();
  HAL_Delay(500U);
}

const U585_Demo U585_Demo_Exp23 = {
  "23",
  "OCTOSPI2 Flash JEDEC ID",
  exp23_init,
  exp23_loop,
};
