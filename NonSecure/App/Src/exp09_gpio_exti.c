#include "u585_board.h"
#include "u585_demo.h"

typedef struct
{
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t button_edges;
  uint8_t previous_button_state;
} U585_Exp09State;

volatile U585_Exp09State g_u585_exp09_state;

static void exp09_init(void)
{
  U585_Board_InitBasicGpio();
  g_u585_exp09_state.iterations = 0U;
  g_u585_exp09_state.tick_ms = HAL_GetTick();
  g_u585_exp09_state.button_edges = 0U;
  g_u585_exp09_state.previous_button_state = U585_Board_IsUserButtonPressed();
}

static void exp09_loop(void)
{
  uint8_t button_state = U585_Board_IsUserButtonPressed();

  g_u585_exp09_state.iterations++;
  g_u585_exp09_state.tick_ms = HAL_GetTick();

  if (button_state != g_u585_exp09_state.previous_button_state)
  {
    g_u585_exp09_state.button_edges++;
    g_u585_exp09_state.previous_button_state = button_state;
    U585_Board_ToggleRedLed();
  }

  HAL_Delay(20U);
}

const U585_Demo U585_Demo_Exp09 = {
  "09",
  "GPIO polling baseline before EXTI migration",
  exp09_init,
  exp09_loop,
};
