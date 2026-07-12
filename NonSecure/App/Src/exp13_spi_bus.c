#include "u585_board.h"
#include "u585_demo.h"
#include "u585_log.h"
#include "u585_spi2.h"

typedef struct
{
  uint32_t magic;
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t spi_ready;
  uint32_t mode;
  uint32_t datasize_bits;
  uint32_t xfer_ok;
  uint32_t tx0;
  uint32_t rx0;
  uint32_t rx1;
  uint32_t rx2;
  uint32_t rx3;
  uint32_t cfg1;
} U585_Exp13State;

volatile U585_Exp13State g_u585_exp13_state;

static void exp13_run_transfer(void)
{
  uint8_t tx[4] = {0x9FU, 0x00U, 0x00U, 0x00U};
  uint8_t rx[4] = {0U, 0U, 0U, 0U};

  g_u585_exp13_state.xfer_ok = 0U;
  g_u585_exp13_state.tx0 = tx[0];
  g_u585_exp13_state.rx0 = 0U;
  g_u585_exp13_state.rx1 = 0U;
  g_u585_exp13_state.rx2 = 0U;
  g_u585_exp13_state.rx3 = 0U;

  if (U585_SPI2_Transfer(tx, rx, 4U, 50U) == HAL_OK)
  {
    g_u585_exp13_state.xfer_ok = 1U;
    g_u585_exp13_state.rx0 = rx[0];
    g_u585_exp13_state.rx1 = rx[1];
    g_u585_exp13_state.rx2 = rx[2];
    g_u585_exp13_state.rx3 = rx[3];
  }

  U585_Log_WriteU32("[U585][13] xfer_ok=", g_u585_exp13_state.xfer_ok);
  U585_Log_WriteU32("[U585][13] tx0=", g_u585_exp13_state.tx0);
  U585_Log_WriteU32("[U585][13] rx0=", g_u585_exp13_state.rx0);
  U585_Log_WriteU32("[U585][13] rx1=", g_u585_exp13_state.rx1);
  U585_Log_WriteU32("[U585][13] rx2=", g_u585_exp13_state.rx2);
  U585_Log_WriteU32("[U585][13] rx3=", g_u585_exp13_state.rx3);
}

static void exp13_init(void)
{
  U585_Board_InitBasicGpio();
  g_u585_exp13_state.magic = 0xA585000DUL;
  g_u585_exp13_state.iterations = 0U;
  g_u585_exp13_state.tick_ms = HAL_GetTick();
  g_u585_exp13_state.mode = 0U;
  g_u585_exp13_state.datasize_bits = 8U;
  g_u585_exp13_state.cfg1 = 0U;
  g_u585_exp13_state.spi_ready = 0U;

  /* Log before touching SPI so a SecureFault is visible on VCP. */
  U585_Log_WriteLine("");
  U585_Log_WriteLine("[U585][13] SPI2 bus (PD1=SCK PD4=MOSI PD3=MISO PB12=NSS)");
  U585_Log_WriteLine("[U585][13] Mode0 8bit soft-NSS (WRLS/EMW3080 path)");
  U585_Log_WriteLine("[U585][13] init_begin");

  g_u585_exp13_state.spi_ready = (U585_SPI2_Init() == HAL_OK) ? 1U : 0U;
  U585_Log_WriteU32("[U585][13] spi_ready=", g_u585_exp13_state.spi_ready);

  if (g_u585_exp13_state.spi_ready != 0U)
  {
    g_u585_exp13_state.cfg1 = SPI2->CFG1;
    U585_Log_WriteU32("[U585][13] mode=", g_u585_exp13_state.mode);
    U585_Log_WriteU32("[U585][13] datasize_bits=", g_u585_exp13_state.datasize_bits);
    U585_Log_WriteU32("[U585][13] cfg1=", g_u585_exp13_state.cfg1);
    exp13_run_transfer();
  }
}

static void exp13_loop(void)
{
  static uint8_t last_button = 0U;
  uint8_t button = U585_Board_IsUserButtonPressed();

  g_u585_exp13_state.iterations++;
  g_u585_exp13_state.tick_ms = HAL_GetTick();

  if ((button != 0U) && (last_button == 0U) && (g_u585_exp13_state.spi_ready != 0U))
  {
    U585_Log_WriteLine("[U585][13] retransfer...");
    exp13_run_transfer();
  }
  last_button = button;

  if ((g_u585_exp13_state.iterations % 4U) == 0U)
  {
    U585_Log_WriteU32("[U585][13] heartbeat=", g_u585_exp13_state.iterations);
  }

  U585_Board_ToggleGreenLed();
  HAL_Delay(500U);
}

const U585_Demo U585_Demo_Exp13 = {
  "13",
  "SPI2 Mode0 bus transfer (WRLS pins)",
  exp13_init,
  exp13_loop,
};
