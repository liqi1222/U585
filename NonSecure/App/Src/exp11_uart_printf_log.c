#include "u585_board.h"
#include "u585_demo.h"
#include "u585_log.h"
#include "u585_usart1.h"

#include <stdio.h>

typedef struct
{
  uint32_t magic;
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t uart_ready;
} U585_Exp11State;

volatile U585_Exp11State g_u585_exp11_state;

static void exp11_init(void)
{
  U585_Board_InitBasicGpio();
  g_u585_exp11_state.magic = 0xA585000BUL;
  g_u585_exp11_state.iterations = 0U;
  g_u585_exp11_state.tick_ms = HAL_GetTick();
  g_u585_exp11_state.uart_ready = U585_USART1_IsReady();

  U585_Log_WriteLine("");
  U585_Log_WriteLine("[U585][11] UART printf/log on NonSecure USART1");
  U585_Log_WriteU32("[U585][11] uart_ready=", g_u585_exp11_state.uart_ready);
  U585_Log_WriteLine("[U585][11] baud=115200 8N1 ST-LINK VCP");
}

static void exp11_loop(void)
{
  char line[96];
  int written;

  g_u585_exp11_state.iterations++;
  g_u585_exp11_state.tick_ms = HAL_GetTick();

  written = snprintf(line, sizeof(line),
                     "[U585][11] printf heartbeat=%lu tick=%lu",
                     (unsigned long)g_u585_exp11_state.iterations,
                     (unsigned long)g_u585_exp11_state.tick_ms);
  if (written > 0)
  {
    U585_Log_WriteLine(line);
  }

  U585_Board_ToggleGreenLed();
  HAL_Delay(1000U);
}

const U585_Demo U585_Demo_Exp11 = {
  "11",
  "UART and printf redirection experiment",
  exp11_init,
  exp11_loop,
};
