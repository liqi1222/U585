#include "u585_board.h"
#include "u585_demo.h"
#include "u585_i2c2.h"
#include "u585_log.h"

/* VL53L5CX default 7-bit address (8-bit 0x52). */
#ifndef U585_VL53L5CX_ADDR7
#define U585_VL53L5CX_ADDR7 0x29U
#endif

/* B-U585I-IOT02A: Mems.VL53_xshut / LPn = PH1 (active high = powered). */
#define U585_VL53_LPN_GPIO_Port GPIOH
#define U585_VL53_LPN_Pin       GPIO_PIN_1

typedef struct
{
  uint32_t magic;
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t i2c_ready;
  uint32_t lpn_high;
  uint32_t probe_ok;
  uint32_t is_alive;
  uint32_t device_id;
  uint32_t revision_id;
  uint32_t uld_ready;
} U585_Exp21State;

volatile U585_Exp21State g_u585_exp21_state;

static HAL_StatusTypeDef vl53_wr8(uint16_t reg, uint8_t value)
{
  return HAL_I2C_Mem_Write(&hi2c2_ns,
                           (uint16_t)(U585_VL53L5CX_ADDR7 << 1),
                           reg,
                           I2C_MEMADD_SIZE_16BIT,
                           &value,
                           1,
                           100);
}

static HAL_StatusTypeDef vl53_rd8(uint16_t reg, uint8_t *value)
{
  return HAL_I2C_Mem_Read(&hi2c2_ns,
                          (uint16_t)(U585_VL53L5CX_ADDR7 << 1),
                          reg,
                          I2C_MEMADD_SIZE_16BIT,
                          value,
                          1,
                          100);
}

static void exp21_power_on(void)
{
  GPIO_InitTypeDef gpio = {0};

  __HAL_RCC_GPIOH_CLK_ENABLE();
  gpio.Pin = U585_VL53_LPN_Pin;
  gpio.Mode = GPIO_MODE_OUTPUT_PP;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(U585_VL53_LPN_GPIO_Port, &gpio);

  /* Boot sequence: ensure low, then raise LPn. */
  HAL_GPIO_WritePin(U585_VL53_LPN_GPIO_Port, U585_VL53_LPN_Pin, GPIO_PIN_RESET);
  HAL_Delay(20U);
  HAL_GPIO_WritePin(U585_VL53_LPN_GPIO_Port, U585_VL53_LPN_Pin, GPIO_PIN_SET);
  HAL_Delay(100U);
  g_u585_exp21_state.lpn_high = 1U;
}

/*
 * Minimal alive check adapted from VL53L5CX ULD vl53l5cx_is_alive()
 * (does not load the ~84 KB ranging firmware).
 */
static uint8_t exp21_is_alive(void)
{
  uint8_t device_id = 0U;
  uint8_t revision_id = 0U;

  g_u585_exp21_state.device_id = 0U;
  g_u585_exp21_state.revision_id = 0U;

  if (vl53_wr8(0x7FFFU, 0x00U) != HAL_OK)
  {
    return 0U;
  }
  if (vl53_wr8(0x0003U, 0x0DU) != HAL_OK)
  {
    return 0U;
  }
  if (vl53_rd8(0x0003U, &device_id) != HAL_OK)
  {
    return 0U;
  }
  if (vl53_rd8(0x0000U, &revision_id) != HAL_OK)
  {
    return 0U;
  }
  (void)vl53_wr8(0x7FFFU, 0x02U);

  g_u585_exp21_state.device_id = device_id;
  g_u585_exp21_state.revision_id = revision_id;
  /* ULD expects (0xF0, 0x02); some boots report 0xF0 on the other register. */
  if (((device_id == 0xF0U) && (revision_id == 0x02U)) ||
      ((revision_id == 0xF0U) && (device_id == 0x02U)) ||
      (device_id == 0xF0U) || (revision_id == 0xF0U))
  {
    return 1U;
  }
  return 0U;
}

static void exp21_probe(void)
{
  g_u585_exp21_state.probe_ok = 0U;
  g_u585_exp21_state.is_alive = 0U;

  if (HAL_I2C_IsDeviceReady(&hi2c2_ns, (uint16_t)(U585_VL53L5CX_ADDR7 << 1), 3, 50) == HAL_OK)
  {
    g_u585_exp21_state.probe_ok = 1U;
    g_u585_exp21_state.is_alive = exp21_is_alive();
  }

  U585_Log_WriteU32("[U585][21] probe_ok=", g_u585_exp21_state.probe_ok);
  U585_Log_WriteU32("[U585][21] is_alive=", g_u585_exp21_state.is_alive);
  U585_Log_WriteU32("[U585][21] device_id=", g_u585_exp21_state.device_id);
  U585_Log_WriteU32("[U585][21] revision_id=", g_u585_exp21_state.revision_id);
}

static void exp21_init(void)
{
  U585_Board_InitBasicGpio();
  g_u585_exp21_state.magic = 0xA5850015UL;
  g_u585_exp21_state.iterations = 0U;
  g_u585_exp21_state.tick_ms = HAL_GetTick();
  g_u585_exp21_state.lpn_high = 0U;
  g_u585_exp21_state.uld_ready = 0U; /* full ULD firmware + ranging later */

  U585_Log_WriteLine("");
  U585_Log_WriteLine("[U585][21] VL53L5CX ToF (I2C2 + PH1 LPn)");
  U585_Log_WriteLine("[U585][21] addr7=0x29 uld_ready=0");

  exp21_power_on();
  U585_Log_WriteU32("[U585][21] lpn_high=", g_u585_exp21_state.lpn_high);

  g_u585_exp21_state.i2c_ready = (U585_I2C2_Init() == HAL_OK) ? 1U : 0U;
  U585_Log_WriteU32("[U585][21] i2c_ready=", g_u585_exp21_state.i2c_ready);

  if (g_u585_exp21_state.i2c_ready != 0U)
  {
    exp21_probe();
  }

  U585_Log_WriteU32("[U585][21] uld_ready=", g_u585_exp21_state.uld_ready);
}

static void exp21_loop(void)
{
  static uint8_t last_button = 0U;
  uint8_t button = U585_Board_IsUserButtonPressed();

  g_u585_exp21_state.iterations++;
  g_u585_exp21_state.tick_ms = HAL_GetTick();

  if ((button != 0U) && (last_button == 0U) && (g_u585_exp21_state.i2c_ready != 0U))
  {
    U585_Log_WriteLine("[U585][21] reprobe...");
    exp21_probe();
  }
  last_button = button;

  if ((g_u585_exp21_state.iterations % 4U) == 0U)
  {
    U585_Log_WriteU32("[U585][21] heartbeat=", g_u585_exp21_state.iterations);
  }

  U585_Board_ToggleGreenLed();
  HAL_Delay(500U);
}

const U585_Demo U585_Demo_Exp21 = {
  "21",
  "VL53L5CX power-on + I2C alive check",
  exp21_init,
  exp21_loop,
};
