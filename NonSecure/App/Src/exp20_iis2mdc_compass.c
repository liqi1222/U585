#include "u585_board.h"
#include "u585_demo.h"
#include "u585_i2c2.h"
#include "u585_log.h"

#ifndef U585_IIS2MDC_ADDR7
#define U585_IIS2MDC_ADDR7 0x1EU
#endif

#define IIS2MDC_WHO_AM_I_REG 0x4FU
#define IIS2MDC_WHO_AM_I_VAL 0x40U
#define IIS2MDC_CFG_REG_A    0x60U
#define IIS2MDC_STATUS_REG   0x67U
#define IIS2MDC_OUTX_L_REG   0x68U

typedef struct
{
  uint32_t magic;
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t i2c_ready;
  uint32_t who_am_i;
  uint32_t who_ok;
  int32_t mx;
  int32_t my;
  int32_t mz;
} U585_Exp20State;

volatile U585_Exp20State g_u585_exp20_state;

static HAL_StatusTypeDef mag_read(uint8_t reg, uint8_t *data, uint16_t len)
{
  return HAL_I2C_Mem_Read(&hi2c2_ns,
                          (uint16_t)(U585_IIS2MDC_ADDR7 << 1),
                          reg,
                          I2C_MEMADD_SIZE_8BIT,
                          data,
                          len,
                          50);
}

static HAL_StatusTypeDef mag_write(uint8_t reg, uint8_t value)
{
  return HAL_I2C_Mem_Write(&hi2c2_ns,
                           (uint16_t)(U585_IIS2MDC_ADDR7 << 1),
                           reg,
                           I2C_MEMADD_SIZE_8BIT,
                           &value,
                           1,
                           50);
}

static int16_t le16(const uint8_t *p)
{
  return (int16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static void exp20_init(void)
{
  uint8_t who = 0U;

  U585_Board_InitBasicGpio();
  g_u585_exp20_state.magic = 0xA5850014UL;
  g_u585_exp20_state.iterations = 0U;
  g_u585_exp20_state.tick_ms = HAL_GetTick();
  g_u585_exp20_state.who_am_i = 0U;
  g_u585_exp20_state.who_ok = 0U;
  g_u585_exp20_state.i2c_ready = (U585_I2C2_Init() == HAL_OK) ? 1U : 0U;

  U585_Log_WriteLine("");
  U585_Log_WriteLine("[U585][20] IIS2MDC magnetometer");
  U585_Log_WriteU32("[U585][20] i2c_ready=", g_u585_exp20_state.i2c_ready);

  if (g_u585_exp20_state.i2c_ready != 0U)
  {
    if (mag_read(IIS2MDC_WHO_AM_I_REG, &who, 1) == HAL_OK)
    {
      g_u585_exp20_state.who_am_i = who;
      g_u585_exp20_state.who_ok = (who == IIS2MDC_WHO_AM_I_VAL) ? 1U : 0U;
    }
    U585_Log_WriteU32("[U585][20] WHO_AM_I=", g_u585_exp20_state.who_am_i);
    U585_Log_WriteU32("[U585][20] who_ok=", g_u585_exp20_state.who_ok);
    /* continuous mode, ODR 10 Hz, temp comp */
    (void)mag_write(IIS2MDC_CFG_REG_A, 0x80U);
  }
}

static void exp20_loop(void)
{
  uint8_t status = 0U;
  uint8_t raw[6] = {0};

  g_u585_exp20_state.iterations++;
  g_u585_exp20_state.tick_ms = HAL_GetTick();

  if ((g_u585_exp20_state.i2c_ready != 0U) && (g_u585_exp20_state.who_ok != 0U))
  {
    if (mag_read(IIS2MDC_STATUS_REG, &status, 1) == HAL_OK)
    {
      if ((status & 0x08U) != 0U)
      {
        if (mag_read(IIS2MDC_OUTX_L_REG, raw, 6) == HAL_OK)
        {
          g_u585_exp20_state.mx = le16(&raw[0]);
          g_u585_exp20_state.my = le16(&raw[2]);
          g_u585_exp20_state.mz = le16(&raw[4]);
          U585_Log_WriteU32("[U585][20] mx=", (uint32_t)(int32_t)g_u585_exp20_state.mx);
          U585_Log_WriteU32("[U585][20] my=", (uint32_t)(int32_t)g_u585_exp20_state.my);
          U585_Log_WriteU32("[U585][20] mz=", (uint32_t)(int32_t)g_u585_exp20_state.mz);
        }
      }
    }
  }

  U585_Board_ToggleGreenLed();
  HAL_Delay(500U);
}

const U585_Demo U585_Demo_Exp20 = {
  "20",
  "IIS2MDC magnetometer",
  exp20_init,
  exp20_loop,
};
