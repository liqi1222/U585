#include "u585_board.h"
#include "u585_demo.h"

typedef struct
{
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t pdm_audio_ready;
} U585_Exp22State;

volatile U585_Exp22State g_u585_exp22_state;

static void exp22_init(void)
{
  U585_Board_InitBasicGpio();
  g_u585_exp22_state.iterations = 0U;
  g_u585_exp22_state.tick_ms = HAL_GetTick();
  g_u585_exp22_state.pdm_audio_ready = 0U;
}

static void exp22_loop(void)
{
  g_u585_exp22_state.iterations++;
  g_u585_exp22_state.tick_ms = HAL_GetTick();
  U585_Board_ToggleGreenLed();
  HAL_Delay(1000U);
}

const U585_Demo U585_Demo_Exp22 = {
  "22",
  "Digital MEMS microphone and PDM capture shell",
  exp22_init,
  exp22_loop,
};
