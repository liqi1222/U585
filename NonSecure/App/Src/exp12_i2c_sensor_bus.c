#include "u585_board.h"
#include "u585_demo.h"
#include "u585_i2c2.h"
#include "u585_log.h"

typedef struct
{
  uint32_t magic;
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t i2c_ready;
  uint32_t found_count;
  uint32_t last_addr7;
  uint32_t scan_done;
} U585_Exp12State;

volatile U585_Exp12State g_u585_exp12_state;

static uint8_t found_addrs[16];

/* U16 RESET is active-low on MB1551; PF11=1 releases it.
 * U27 LPn / VL53_xshut is PH1, high to leave shutdown.
 */
static void exp12_enable_gated_devices(void)
{
  GPIO_InitTypeDef gpio = {0};

  __HAL_RCC_GPIOF_CLK_ENABLE();
  gpio.Pin = GPIO_PIN_11;
  gpio.Mode = GPIO_MODE_OUTPUT_PP;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOF, &gpio);
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_11, GPIO_PIN_SET);
  HAL_Delay(10U);

  __HAL_RCC_GPIOH_CLK_ENABLE();
  gpio.Pin = GPIO_PIN_1;
  HAL_GPIO_Init(GPIOH, &gpio);
  HAL_GPIO_WritePin(GPIOH, GPIO_PIN_1, GPIO_PIN_RESET);
  HAL_Delay(20U);
  HAL_GPIO_WritePin(GPIOH, GPIO_PIN_1, GPIO_PIN_SET);
  HAL_Delay(100U);
}

static void exp12_scan(void)
{
  uint8_t addr7;
  uint32_t count = 0U;

  g_u585_exp12_state.found_count = 0U;
  g_u585_exp12_state.last_addr7 = 0U;

  for (addr7 = 1U; addr7 < 0x78U; addr7++)
  {
    if (HAL_I2C_IsDeviceReady(&hi2c2_ns, (uint16_t)(addr7 << 1), 2, 20) == HAL_OK)
    {
      if (count < (sizeof(found_addrs) / sizeof(found_addrs[0])))
      {
        found_addrs[count] = addr7;
      }
      count++;
      g_u585_exp12_state.last_addr7 = addr7;
      U585_Log_WriteU32("[U585][12] found_addr7=", addr7);
    }
  }

  g_u585_exp12_state.found_count = count;
  g_u585_exp12_state.scan_done = 1U;
  U585_Log_WriteU32("[U585][12] found_count=", count);
}

static void exp12_init(void)
{
  U585_Board_InitBasicGpio();
  g_u585_exp12_state.magic = 0xA585000CUL;
  g_u585_exp12_state.iterations = 0U;
  g_u585_exp12_state.tick_ms = HAL_GetTick();
  g_u585_exp12_state.scan_done = 0U;
  g_u585_exp12_state.i2c_ready = (U585_I2C2_Init() == HAL_OK) ? 1U : 0U;

  exp12_enable_gated_devices();

  U585_Log_WriteLine("");
  U585_Log_WriteLine("[U585][12] I2C2 bus scan (PH4=SCL PH5=SDA)");
  U585_Log_WriteLine("[U585][12] stsafe_reset PF11=1; vl53_lpn PH1=1");
  U585_Log_WriteU32("[U585][12] i2c_ready=", g_u585_exp12_state.i2c_ready);

  if (g_u585_exp12_state.i2c_ready != 0U)
  {
    exp12_scan();
  }
}

static void exp12_loop(void)
{
  static uint8_t last_button = 0U;
  uint8_t button = U585_Board_IsUserButtonPressed();

  g_u585_exp12_state.iterations++;
  g_u585_exp12_state.tick_ms = HAL_GetTick();

  if ((button != 0U) && (last_button == 0U) && (g_u585_exp12_state.i2c_ready != 0U))
  {
    U585_Log_WriteLine("[U585][12] rescan...");
    exp12_scan();
  }
  last_button = button;

  U585_Board_ToggleGreenLed();
  HAL_Delay(500U);
}

const U585_Demo U585_Demo_Exp12 = {
  "12",
  "I2C sensor bus scan",
  exp12_init,
  exp12_loop,
};
