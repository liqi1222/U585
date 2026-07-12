#include "u585_board.h"
#include "u585_demo.h"
#include "u585_emw3080.h"
#include "u585_log.h"
#include "u585_spi2.h"

typedef struct
{
  uint32_t magic;
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t emw_ready;
  uint32_t spi_ready;
  uint32_t chip_en;
  uint32_t flow;
  uint32_t notify;
  uint32_t flow_ok;
  uint32_t xfer_ok;
  uint32_t rx0;
  uint32_t rx1;
  uint32_t rx2;
  uint32_t rx3;
} U585_Exp27State;

volatile U585_Exp27State g_u585_exp27_state;

static void exp27_sample_pins(void)
{
  g_u585_exp27_state.chip_en = U585_EMW3080_ReadChipEn();
  g_u585_exp27_state.flow = U585_EMW3080_ReadFlow();
  g_u585_exp27_state.notify = U585_EMW3080_ReadNotify();
  g_u585_exp27_state.flow_ok = (g_u585_exp27_state.flow != 0U) ? 1U : 0U;

  U585_Log_WriteU32("[U585][27] chip_en=", g_u585_exp27_state.chip_en);
  U585_Log_WriteU32("[U585][27] flow=", g_u585_exp27_state.flow);
  U585_Log_WriteU32("[U585][27] notify=", g_u585_exp27_state.notify);
  U585_Log_WriteU32("[U585][27] flow_ok=", g_u585_exp27_state.flow_ok);
}

static void exp27_spi_probe(void)
{
  uint8_t tx[4] = {0x05U, 0x00U, 0x00U, 0x00U};
  uint8_t rx[4] = {0U, 0U, 0U, 0U};

  g_u585_exp27_state.xfer_ok = 0U;
  g_u585_exp27_state.rx0 = 0U;
  g_u585_exp27_state.rx1 = 0U;
  g_u585_exp27_state.rx2 = 0U;
  g_u585_exp27_state.rx3 = 0U;

  if (U585_SPI2_Transfer(tx, rx, 4U, 50U) == HAL_OK)
  {
    g_u585_exp27_state.xfer_ok = 1U;
    g_u585_exp27_state.rx0 = rx[0];
    g_u585_exp27_state.rx1 = rx[1];
    g_u585_exp27_state.rx2 = rx[2];
    g_u585_exp27_state.rx3 = rx[3];
  }

  U585_Log_WriteU32("[U585][27] xfer_ok=", g_u585_exp27_state.xfer_ok);
  U585_Log_WriteU32("[U585][27] rx0=", g_u585_exp27_state.rx0);
  U585_Log_WriteU32("[U585][27] rx1=", g_u585_exp27_state.rx1);
  U585_Log_WriteU32("[U585][27] rx2=", g_u585_exp27_state.rx2);
  U585_Log_WriteU32("[U585][27] rx3=", g_u585_exp27_state.rx3);
}

static void exp27_init(void)
{
  U585_Board_InitBasicGpio();
  g_u585_exp27_state.magic = 0xA585001BUL;
  g_u585_exp27_state.iterations = 0U;
  g_u585_exp27_state.tick_ms = HAL_GetTick();

  U585_Log_WriteLine("");
  U585_Log_WriteLine("[U585][27] EMW3080 bring-up (SPI2 + Chip_En/FLOW/NOTIFY)");
  U585_Log_WriteLine("[U585][27] PF15=Chip_En PG15=FLOW PD14=NOTIFY");
  U585_Log_WriteLine("[U585][27] mx_wifi / join-AP pending");

  g_u585_exp27_state.emw_ready = (U585_EMW3080_Init() == HAL_OK) ? 1U : 0U;
  U585_Log_WriteU32("[U585][27] emw_ready=", g_u585_exp27_state.emw_ready);

  g_u585_exp27_state.spi_ready = (U585_SPI2_Init() == HAL_OK) ? 1U : 0U;
  U585_Log_WriteU32("[U585][27] spi_ready=", g_u585_exp27_state.spi_ready);

  if (g_u585_exp27_state.emw_ready != 0U)
  {
    exp27_sample_pins();
  }

  if (g_u585_exp27_state.spi_ready != 0U)
  {
    exp27_spi_probe();
    /* SPI may wake NOTIFY; sample again. */
    if (g_u585_exp27_state.emw_ready != 0U)
    {
      exp27_sample_pins();
    }
  }
}

static void exp27_loop(void)
{
  g_u585_exp27_state.iterations++;
  g_u585_exp27_state.tick_ms = HAL_GetTick();

  if ((g_u585_exp27_state.emw_ready != 0U) && ((g_u585_exp27_state.iterations % 2U) == 0U))
  {
    exp27_sample_pins();
  }

  if ((g_u585_exp27_state.iterations % 4U) == 0U)
  {
    U585_Log_WriteU32("[U585][27] heartbeat=", g_u585_exp27_state.iterations);
  }

  U585_Board_ToggleGreenLed();
  HAL_Delay(500U);
}

const U585_Demo U585_Demo_Exp27 = {
  "27",
  "EMW3080 Chip_En + SPI2 probe",
  exp27_init,
  exp27_loop,
};
