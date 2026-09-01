#include "u585_board.h"
#include "u585_demo.h"
#include "u585_i2c2.h"
#include "u585_log.h"

#ifndef U585_ISM330_ADDR7
#define U585_ISM330_ADDR7 0x6BU
#endif

#define ISM330_WHO_AM_I_REG 0x0FU
#define ISM330_WHO_AM_I_VAL 0x6BU
#define ISM330_CTRL1_XL     0x10U
#define ISM330_CTRL2_G      0x11U
#define ISM330_STATUS_REG   0x1EU
#define ISM330_OUTX_L_G     0x22U

typedef struct
{
  uint32_t magic;
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t i2c_ready;
  uint32_t who_am_i;
  uint32_t who_ok;
  int32_t gx;
  int32_t gy;
  int32_t gz;
  int32_t ax;
  int32_t ay;
  int32_t az;
} U585_Exp19State;

volatile U585_Exp19State g_u585_exp19_state;

static HAL_StatusTypeDef ism_read(uint8_t reg, uint8_t *data, uint16_t len)
{
  return HAL_I2C_Mem_Read(&hi2c2_ns,
                          (uint16_t)(U585_ISM330_ADDR7 << 1),
                          reg,
                          I2C_MEMADD_SIZE_8BIT,
                          data,
                          len,
                          50);
}

static HAL_StatusTypeDef ism_write(uint8_t reg, uint8_t value)
{
  return HAL_I2C_Mem_Write(&hi2c2_ns,
                           (uint16_t)(U585_ISM330_ADDR7 << 1),
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

static void exp19_init(void)
{
  uint8_t who = 0U;

  U585_Board_InitBasicGpio();
  g_u585_exp19_state.magic = 0xA5850013UL;
  g_u585_exp19_state.iterations = 0U;
  g_u585_exp19_state.tick_ms = HAL_GetTick();
  g_u585_exp19_state.who_am_i = 0U;
  g_u585_exp19_state.who_ok = 0U;
  g_u585_exp19_state.i2c_ready = (U585_I2C2_Init() == HAL_OK) ? 1U : 0U;

  U585_Log_WriteLine("");
  U585_Log_WriteLine("[U585][19] ISM330DHCX IMU");
  U585_Log_WriteU32("[U585][19] i2c_ready=", g_u585_exp19_state.i2c_ready);

  if (g_u585_exp19_state.i2c_ready != 0U)
  {
    if (ism_read(ISM330_WHO_AM_I_REG, &who, 1) == HAL_OK)
    {
      g_u585_exp19_state.who_am_i = who;
      g_u585_exp19_state.who_ok = (who == ISM330_WHO_AM_I_VAL) ? 1U : 0U;
    }
    U585_Log_WriteU32("[U585][19] WHO_AM_I=", g_u585_exp19_state.who_am_i);
    U585_Log_WriteU32("[U585][19] who_ok=", g_u585_exp19_state.who_ok);
    /* XL 104 Hz +-2g; G 104 Hz 250 dps */
    (void)ism_write(ISM330_CTRL1_XL, 0x40U);
    (void)ism_write(ISM330_CTRL2_G, 0x40U);
  }
}

static void exp19_loop(void)
{
  uint8_t status = 0U;
  uint8_t raw[12] = {0};

  g_u585_exp19_state.iterations++;
  g_u585_exp19_state.tick_ms = HAL_GetTick();

  if ((g_u585_exp19_state.i2c_ready != 0U) && (g_u585_exp19_state.who_ok != 0U))
  {
    if (ism_read(ISM330_STATUS_REG, &status, 1) == HAL_OK)
    {
      if ((status & 0x03U) != 0U)
      {
        if (ism_read(ISM330_OUTX_L_G, raw, 12) == HAL_OK)
        {
          g_u585_exp19_state.gx = le16(&raw[0]);
          g_u585_exp19_state.gy = le16(&raw[2]);
          g_u585_exp19_state.gz = le16(&raw[4]);
          g_u585_exp19_state.ax = le16(&raw[6]);
          g_u585_exp19_state.ay = le16(&raw[8]);
          g_u585_exp19_state.az = le16(&raw[10]);
          U585_Log_WriteU32("[U585][19] ax=", (uint32_t)(int32_t)g_u585_exp19_state.ax);
          U585_Log_WriteU32("[U585][19] ay=", (uint32_t)(int32_t)g_u585_exp19_state.ay);
          U585_Log_WriteU32("[U585][19] az=", (uint32_t)(int32_t)g_u585_exp19_state.az);
          U585_Log_WriteU32("[U585][19] gx=", (uint32_t)(int32_t)g_u585_exp19_state.gx);
        }
      }
    }
  }

  U585_Board_ToggleGreenLed();
  HAL_Delay(500U);
}

const U585_Demo U585_Demo_Exp19 = {
  "19",
  "ISM330DHCX 6-axis IMU",
  exp19_init,
  exp19_loop,
};
