#include "u585_board.h"
#include "u585_demo.h"

typedef struct
{
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t gpdma_nonsecure_ready;
} U585_Exp16State;

volatile U585_Exp16State g_u585_exp16_state;

static void exp16_init(void)
{
  U585_Board_InitBasicGpio();
  g_u585_exp16_state.iterations = 0U;
  g_u585_exp16_state.tick_ms = HAL_GetTick();
  g_u585_exp16_state.gpdma_nonsecure_ready = 0U;
}

static void exp16_loop(void)
{
  g_u585_exp16_state.iterations++;
  g_u585_exp16_state.tick_ms = HAL_GetTick();
  U585_Board_ToggleGreenLed();
  HAL_Delay(1000U);
}

const U585_Demo U585_Demo_Exp16 = {
  "16",
  "GPDMA memory-transfer experiment shell",
  exp16_init,
  exp16_loop,
};
