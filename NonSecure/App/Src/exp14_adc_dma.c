#include "u585_board.h"
#include "u585_demo.h"

typedef struct
{
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t adc_dma_nonsecure_ready;
} U585_Exp14State;

volatile U585_Exp14State g_u585_exp14_state;

static void exp14_init(void)
{
  U585_Board_InitBasicGpio();
  g_u585_exp14_state.iterations = 0U;
  g_u585_exp14_state.tick_ms = HAL_GetTick();
  g_u585_exp14_state.adc_dma_nonsecure_ready = 0U;
}

static void exp14_loop(void)
{
  g_u585_exp14_state.iterations++;
  g_u585_exp14_state.tick_ms = HAL_GetTick();
  U585_Board_ToggleGreenLed();
  HAL_Delay(1000U);
}

const U585_Demo U585_Demo_Exp14 = {
  "14",
  "ADC sampling and DMA experiment shell",
  exp14_init,
  exp14_loop,
};
