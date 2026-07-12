#include "u585_board.h"
#include "u585_demo.h"
#include "u585_log.h"

static uint32_t blink_delay_ms = 500U;
static uint8_t last_button_pressed = 0U;
static uint32_t loop_count = 0U;
static GPIO_PinState red_led_state = GPIO_PIN_SET;
static GPIO_PinState green_led_state = GPIO_PIN_RESET;

typedef struct
{
  uint32_t magic;
  uint32_t init_count;
  uint32_t loop_count;
  uint32_t tick_ms;
  uint32_t blink_delay_ms;
  uint32_t button_pressed;
  uint32_t red_led_state;
  uint32_t green_led_state;
  uint32_t log_checkpoint;
} U585_Exp03State;

volatile U585_Exp03State g_u585_exp03_state;

static void exp03_log_led_state(void)
{
  U585_Log_WriteU32("[U585][03] led_state=", ((uint32_t)red_led_state << 1U) | (uint32_t)green_led_state);
}

static void exp03_init(void)
{
  U585_Board_InitBasicGpio();
  loop_count = 0U;
  red_led_state = GPIO_PIN_SET;
  green_led_state = GPIO_PIN_RESET;
  U585_Board_SetRedLed(red_led_state);
  U585_Board_SetGreenLed(green_led_state);
  last_button_pressed = U585_Board_IsUserButtonPressed();

  g_u585_exp03_state.magic = 0xA5850003UL;
  g_u585_exp03_state.init_count++;
  g_u585_exp03_state.loop_count = 0U;
  g_u585_exp03_state.tick_ms = HAL_GetTick();
  g_u585_exp03_state.blink_delay_ms = blink_delay_ms;
  g_u585_exp03_state.button_pressed = last_button_pressed;
  g_u585_exp03_state.red_led_state = (uint32_t)red_led_state;
  g_u585_exp03_state.green_led_state = (uint32_t)green_led_state;
  g_u585_exp03_state.log_checkpoint = 1U;

  U585_Log_WriteLine("");
  g_u585_exp03_state.log_checkpoint = 2U;
  U585_Log_WriteLine("[U585][03] LED/button demo start");
  g_u585_exp03_state.log_checkpoint = 3U;
  U585_Log_WriteLine("[U585][03] USART1 VCP log via Secure NSC bridge");
  g_u585_exp03_state.log_checkpoint = 4U;
  U585_Log_WriteU32("[U585][03] initial_button=", last_button_pressed);
  g_u585_exp03_state.log_checkpoint = 5U;
  exp03_log_led_state();
  g_u585_exp03_state.log_checkpoint = 6U;
}

static void exp03_loop(void)
{
  uint8_t button_pressed = U585_Board_IsUserButtonPressed();

  loop_count++;
  g_u585_exp03_state.loop_count = loop_count;
  g_u585_exp03_state.tick_ms = HAL_GetTick();
  g_u585_exp03_state.button_pressed = button_pressed;
  g_u585_exp03_state.blink_delay_ms = blink_delay_ms;

  if ((button_pressed != 0U) && (last_button_pressed == 0U))
  {
    blink_delay_ms = (blink_delay_ms == 500U) ? 100U : 500U;
    g_u585_exp03_state.blink_delay_ms = blink_delay_ms;
    g_u585_exp03_state.log_checkpoint = 7U;
    U585_Log_WriteU32("[U585][03] button_edge_delay_ms=", blink_delay_ms);
    g_u585_exp03_state.log_checkpoint = 8U;
  }

  last_button_pressed = button_pressed;

  if (red_led_state == GPIO_PIN_SET)
  {
    red_led_state = GPIO_PIN_RESET;
    green_led_state = GPIO_PIN_SET;
  }
  else
  {
    red_led_state = GPIO_PIN_SET;
    green_led_state = GPIO_PIN_RESET;
  }
  U585_Board_SetRedLed(red_led_state);
  U585_Board_SetGreenLed(green_led_state);
  g_u585_exp03_state.red_led_state = (uint32_t)red_led_state;
  g_u585_exp03_state.green_led_state = (uint32_t)green_led_state;
  g_u585_exp03_state.log_checkpoint = 9U;
  exp03_log_led_state();

  if ((loop_count % 10U) == 0U)
  {
    g_u585_exp03_state.log_checkpoint = 10U;
    U585_Log_WriteU32("[U585][03] heartbeat=", loop_count);
    g_u585_exp03_state.log_checkpoint = 11U;
  }
  HAL_Delay(blink_delay_ms);
}

const U585_Demo U585_Demo_Exp03LedButton = {
  "03",
  "CubeMX VS Code baseline: LED and button",
  exp03_init,
  exp03_loop,
};
