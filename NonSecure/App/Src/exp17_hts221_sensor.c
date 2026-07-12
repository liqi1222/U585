#include "u585_board.h"
#include "u585_demo.h"
#include "u585_i2c2.h"
#include "u585_log.h"

/* HTS221 7-bit address on B-U585I-IOT02A */
#ifndef U585_HTS221_ADDR7
#define U585_HTS221_ADDR7 0x5FU
#endif

#define HTS221_WHO_AM_I_REG 0x0FU
#define HTS221_WHO_AM_I_VAL 0xBCU
#define HTS221_CTRL_REG1    0x20U
#define HTS221_STATUS_REG   0x27U
#define HTS221_HUMIDITY_OUT_L 0x28U
#define HTS221_TEMP_OUT_L     0x2AU

typedef struct
{
  uint32_t magic;
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t i2c_ready;
  uint32_t who_am_i;
  uint32_t who_ok;
  uint32_t raw_temp;
  uint32_t raw_hum;
} U585_Exp17State;

volatile U585_Exp17State g_u585_exp17_state;

static HAL_StatusTypeDef hts221_read(uint8_t reg, uint8_t *data, uint16_t len)
{
  return HAL_I2C_Mem_Read(&hi2c2_ns,
                          (uint16_t)(U585_HTS221_ADDR7 << 1),
                          reg,
                          I2C_MEMADD_SIZE_8BIT,
                          data,
                          len,
                          50);
}

static HAL_StatusTypeDef hts221_write(uint8_t reg, uint8_t value)
{
  return HAL_I2C_Mem_Write(&hi2c2_ns,
                           (uint16_t)(U585_HTS221_ADDR7 << 1),
                           reg,
                           I2C_MEMADD_SIZE_8BIT,
                           &value,
                           1,
                           50);
}

static void exp17_init(void)
{
  uint8_t who = 0U;

  U585_Board_InitBasicGpio();
  g_u585_exp17_state.magic = 0xA5850011UL;
  g_u585_exp17_state.iterations = 0U;
  g_u585_exp17_state.tick_ms = HAL_GetTick();
  g_u585_exp17_state.who_am_i = 0U;
  g_u585_exp17_state.who_ok = 0U;
  g_u585_exp17_state.raw_temp = 0U;
  g_u585_exp17_state.raw_hum = 0U;
  g_u585_exp17_state.i2c_ready = (U585_I2C2_Init() == HAL_OK) ? 1U : 0U;

  U585_Log_WriteLine("");
  U585_Log_WriteLine("[U585][17] HTS221 humidity/temp");
  U585_Log_WriteU32("[U585][17] i2c_ready=", g_u585_exp17_state.i2c_ready);

  if (g_u585_exp17_state.i2c_ready != 0U)
  {
    if (hts221_read(HTS221_WHO_AM_I_REG, &who, 1) == HAL_OK)
    {
      g_u585_exp17_state.who_am_i = who;
      g_u585_exp17_state.who_ok = (who == HTS221_WHO_AM_I_VAL) ? 1U : 0U;
    }
    U585_Log_WriteU32("[U585][17] WHO_AM_I=", g_u585_exp17_state.who_am_i);
    U585_Log_WriteU32("[U585][17] who_ok=", g_u585_exp17_state.who_ok);

    /* PD=1, BDU=1, ODR=1 Hz */
    (void)hts221_write(HTS221_CTRL_REG1, 0x85U);
  }
}

static void exp17_loop(void)
{
  uint8_t status = 0U;
  uint8_t raw[4] = {0};

  g_u585_exp17_state.iterations++;
  g_u585_exp17_state.tick_ms = HAL_GetTick();

  if ((g_u585_exp17_state.i2c_ready != 0U) && (g_u585_exp17_state.who_ok != 0U))
  {
    if (hts221_read(HTS221_STATUS_REG, &status, 1) == HAL_OK)
    {
      if ((status & 0x03U) != 0U)
      {
        /* auto-increment bit for multi-byte read */
        if (hts221_read((uint8_t)(HTS221_HUMIDITY_OUT_L | 0x80U), raw, 4) == HAL_OK)
        {
          g_u585_exp17_state.raw_hum =
              (uint32_t)raw[0] | ((uint32_t)raw[1] << 8);
          g_u585_exp17_state.raw_temp =
              (uint32_t)raw[2] | ((uint32_t)raw[3] << 8);
          U585_Log_WriteU32("[U585][17] raw_hum=", g_u585_exp17_state.raw_hum);
          U585_Log_WriteU32("[U585][17] raw_temp=", g_u585_exp17_state.raw_temp);
        }
      }
    }
  }

  U585_Board_ToggleGreenLed();
  HAL_Delay(1000U);
}

const U585_Demo U585_Demo_Exp17 = {
  "17",
  "HTS221 humidity and temperature sensor",
  exp17_init,
  exp17_loop,
};
