#include "u585_board.h"
#include "u585_demo.h"
#include "u585_i2c2.h"
#include "u585_log.h"

#ifndef U585_LPS22HH_ADDR7
#define U585_LPS22HH_ADDR7 0x5DU
#endif

#define LPS22HH_WHO_AM_I_REG 0x0FU
#define LPS22HH_WHO_AM_I_VAL 0xB3U
#define LPS22HH_CTRL_REG1    0x10U
#define LPS22HH_STATUS       0x27U
#define LPS22HH_PRESS_OUT_XL 0x28U

typedef struct
{
  uint32_t magic;
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t i2c_ready;
  uint32_t who_am_i;
  uint32_t who_ok;
  uint32_t raw_press;
  uint32_t raw_temp;
} U585_Exp18State;

volatile U585_Exp18State g_u585_exp18_state;

static HAL_StatusTypeDef lps_read(uint8_t reg, uint8_t *data, uint16_t len)
{
  return HAL_I2C_Mem_Read(&hi2c2_ns,
                          (uint16_t)(U585_LPS22HH_ADDR7 << 1),
                          reg,
                          I2C_MEMADD_SIZE_8BIT,
                          data,
                          len,
                          50);
}

static HAL_StatusTypeDef lps_write(uint8_t reg, uint8_t value)
{
  return HAL_I2C_Mem_Write(&hi2c2_ns,
                           (uint16_t)(U585_LPS22HH_ADDR7 << 1),
                           reg,
                           I2C_MEMADD_SIZE_8BIT,
                           &value,
                           1,
                           50);
}

static void exp18_init(void)
{
  uint8_t who = 0U;

  U585_Board_InitBasicGpio();
  g_u585_exp18_state.magic = 0xA5850012UL;
  g_u585_exp18_state.iterations = 0U;
  g_u585_exp18_state.tick_ms = HAL_GetTick();
  g_u585_exp18_state.who_am_i = 0U;
  g_u585_exp18_state.who_ok = 0U;
  g_u585_exp18_state.raw_press = 0U;
  g_u585_exp18_state.raw_temp = 0U;
  g_u585_exp18_state.i2c_ready = (U585_I2C2_Init() == HAL_OK) ? 1U : 0U;

  U585_Log_WriteLine("");
  U585_Log_WriteLine("[U585][18] LPS22HH pressure/temp");
  U585_Log_WriteU32("[U585][18] i2c_ready=", g_u585_exp18_state.i2c_ready);

  if (g_u585_exp18_state.i2c_ready != 0U)
  {
    if (lps_read(LPS22HH_WHO_AM_I_REG, &who, 1) == HAL_OK)
    {
      g_u585_exp18_state.who_am_i = who;
      g_u585_exp18_state.who_ok = (who == LPS22HH_WHO_AM_I_VAL) ? 1U : 0U;
    }
    U585_Log_WriteU32("[U585][18] WHO_AM_I=", g_u585_exp18_state.who_am_i);
    U585_Log_WriteU32("[U585][18] who_ok=", g_u585_exp18_state.who_ok);
    /* ODR=10 Hz, LPF enabled */
    (void)lps_write(LPS22HH_CTRL_REG1, 0x20U);
  }
}

static void exp18_loop(void)
{
  uint8_t status = 0U;
  uint8_t raw[5] = {0};

  g_u585_exp18_state.iterations++;
  g_u585_exp18_state.tick_ms = HAL_GetTick();

  if ((g_u585_exp18_state.i2c_ready != 0U) && (g_u585_exp18_state.who_ok != 0U))
  {
    if (lps_read(LPS22HH_STATUS, &status, 1) == HAL_OK)
    {
      if ((status & 0x03U) != 0U)
      {
        if (lps_read((uint8_t)(LPS22HH_PRESS_OUT_XL | 0x80U), raw, 5) == HAL_OK)
        {
          g_u585_exp18_state.raw_press =
              (uint32_t)raw[0] | ((uint32_t)raw[1] << 8) | ((uint32_t)raw[2] << 16);
          g_u585_exp18_state.raw_temp =
              (uint32_t)raw[3] | ((uint32_t)raw[4] << 8);
          U585_Log_WriteU32("[U585][18] raw_press=", g_u585_exp18_state.raw_press);
          U585_Log_WriteU32("[U585][18] raw_temp=", g_u585_exp18_state.raw_temp);
        }
      }
    }
  }

  U585_Board_ToggleGreenLed();
  HAL_Delay(1000U);
}

const U585_Demo U585_Demo_Exp18 = {
  "18",
  "LPS22HH pressure sensor",
  exp18_init,
  exp18_loop,
};
