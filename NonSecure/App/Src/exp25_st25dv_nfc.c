#include "u585_board.h"
#include "u585_demo.h"
#include "u585_i2c2.h"
#include "u585_log.h"

/*
 * Article 25 targets ST25DV NFC, but B-U585I-IOT02A BSP wires M24256
 * I2C EEPROM at 8-bit address 0xAC (7-bit 0x56). Probe both.
 */
#define U585_ST25DV_USER_ADDR7   0x53U
#define U585_ST25DV_SYS_ADDR7    0x57U
#define U585_M24256_ADDR7        0x56U
#define U585_EEPROM_TEST_ADDR    0x0010U

typedef struct
{
  uint32_t magic;
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t i2c_ready;
  uint32_t st25_user_ok;
  uint32_t st25_sys_ok;
  uint32_t eeprom_ok;
  uint32_t rw_ok;
  uint32_t readback;
} U585_Exp25State;

volatile U585_Exp25State g_u585_exp25_state;

static uint8_t i2c_probe7(uint8_t addr7)
{
  return (HAL_I2C_IsDeviceReady(&hi2c2_ns, (uint16_t)(addr7 << 1), 2, 50) == HAL_OK) ? 1U : 0U;
}

static HAL_StatusTypeDef eeprom_wait_ready(void)
{
  uint32_t trials = 0U;
  while (trials < 100U)
  {
    if (HAL_I2C_IsDeviceReady(&hi2c2_ns, (uint16_t)(U585_M24256_ADDR7 << 1), 1, 20) == HAL_OK)
    {
      return HAL_OK;
    }
    trials++;
    HAL_Delay(1U);
  }
  return HAL_TIMEOUT;
}

static HAL_StatusTypeDef eeprom_write_byte(uint16_t mem_addr, uint8_t value)
{
  if (HAL_I2C_Mem_Write(&hi2c2_ns,
                        (uint16_t)(U585_M24256_ADDR7 << 1),
                        mem_addr,
                        I2C_MEMADD_SIZE_16BIT,
                        &value,
                        1,
                        50) != HAL_OK)
  {
    return HAL_ERROR;
  }
  return eeprom_wait_ready();
}

static HAL_StatusTypeDef eeprom_read_byte(uint16_t mem_addr, uint8_t *value)
{
  return HAL_I2C_Mem_Read(&hi2c2_ns,
                          (uint16_t)(U585_M24256_ADDR7 << 1),
                          mem_addr,
                          I2C_MEMADD_SIZE_16BIT,
                          value,
                          1,
                          50);
}

static void exp25_probe(void)
{
  uint8_t pattern = (uint8_t)(HAL_GetTick() & 0xFFU);
  uint8_t readback = 0U;
  uint8_t saved = 0U;

  g_u585_exp25_state.st25_user_ok = i2c_probe7(U585_ST25DV_USER_ADDR7);
  g_u585_exp25_state.st25_sys_ok = i2c_probe7(U585_ST25DV_SYS_ADDR7);
  g_u585_exp25_state.eeprom_ok = i2c_probe7(U585_M24256_ADDR7);
  g_u585_exp25_state.rw_ok = 0U;
  g_u585_exp25_state.readback = 0U;

  U585_Log_WriteU32("[U585][25] st25_user_ok=", g_u585_exp25_state.st25_user_ok);
  U585_Log_WriteU32("[U585][25] st25_sys_ok=", g_u585_exp25_state.st25_sys_ok);
  U585_Log_WriteU32("[U585][25] eeprom_ok=", g_u585_exp25_state.eeprom_ok);

  if (g_u585_exp25_state.eeprom_ok != 0U)
  {
    /* Preserve original byte, write pattern, verify, restore. */
    if (eeprom_read_byte(U585_EEPROM_TEST_ADDR, &saved) == HAL_OK)
    {
      if ((eeprom_write_byte(U585_EEPROM_TEST_ADDR, pattern) == HAL_OK) &&
          (eeprom_read_byte(U585_EEPROM_TEST_ADDR, &readback) == HAL_OK) &&
          (readback == pattern))
      {
        g_u585_exp25_state.rw_ok = 1U;
        g_u585_exp25_state.readback = readback;
      }
      (void)eeprom_write_byte(U585_EEPROM_TEST_ADDR, saved);
    }
  }

  U585_Log_WriteU32("[U585][25] rw_ok=", g_u585_exp25_state.rw_ok);
  U585_Log_WriteU32("[U585][25] readback=", g_u585_exp25_state.readback);
}

static void exp25_init(void)
{
  U585_Board_InitBasicGpio();
  g_u585_exp25_state.magic = 0xA5850019UL;
  g_u585_exp25_state.iterations = 0U;
  g_u585_exp25_state.tick_ms = HAL_GetTick();

  U585_Log_WriteLine("");
  U585_Log_WriteLine("[U585][25] NFC article: probe ST25DV + board M24256 EEPROM");
  U585_Log_WriteLine("[U585][25] ST25DV expect 0x53/0x57; EEPROM 0x56 (0xAC)");

  g_u585_exp25_state.i2c_ready = (U585_I2C2_Init() == HAL_OK) ? 1U : 0U;
  U585_Log_WriteU32("[U585][25] i2c_ready=", g_u585_exp25_state.i2c_ready);

  if (g_u585_exp25_state.i2c_ready != 0U)
  {
    exp25_probe();
  }
}

static void exp25_loop(void)
{
  static uint8_t last_button = 0U;
  uint8_t button = U585_Board_IsUserButtonPressed();

  g_u585_exp25_state.iterations++;
  g_u585_exp25_state.tick_ms = HAL_GetTick();

  if ((button != 0U) && (last_button == 0U) && (g_u585_exp25_state.i2c_ready != 0U))
  {
    U585_Log_WriteLine("[U585][25] re-probe...");
    exp25_probe();
  }
  last_button = button;

  if ((g_u585_exp25_state.iterations % 4U) == 0U)
  {
    U585_Log_WriteU32("[U585][25] heartbeat=", g_u585_exp25_state.iterations);
  }

  U585_Board_ToggleGreenLed();
  HAL_Delay(500U);
}

const U585_Demo U585_Demo_Exp25 = {
  "25",
  "I2C EEPROM / ST25DV probe",
  exp25_init,
  exp25_loop,
};
