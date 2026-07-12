#include "u585_board.h"
#include "u585_demo.h"
#include "u585_log.h"

typedef struct
{
  uint32_t magic;
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t wifi_ready;
  uint32_t freertos_ready;
  uint32_t mqtt_ready;
  uint32_t connect_pkt_len;
  uint32_t connect_pkt_ok;
} U585_Exp29State;

volatile U585_Exp29State g_u585_exp29_state;

/*
 * Minimal MQTT CONNECT (v3.1.1) framing without a network stack.
 * Fixed header + remaining length + variable header + empty client-id payload
 * is enough to prove packet layout knowledge before Wi-Fi/RTOS land.
 */
static uint32_t exp29_build_connect(uint8_t *out, uint32_t out_cap)
{
  /* CONNECT: type=1, flags=0; remaining length for protocol name MQTT + level + flags + keepalive + client id "". */
  static const uint8_t k_connect[] = {
    0x10U,       /* Fixed: CONNECT */
    0x0CU,       /* Remaining length = 12 */
    0x00U, 0x04U, 'M', 'Q', 'T', 'T',
    0x04U,       /* Protocol level 4 = 3.1.1 */
    0x02U,       /* Clean session */
    0x00U, 0x3CU,/* Keep Alive 60 s */
    0x00U, 0x00U /* Client ID length 0 */
  };

  if ((out == NULL) || (out_cap < sizeof(k_connect)))
  {
    return 0U;
  }
  for (uint32_t i = 0U; i < sizeof(k_connect); i++)
  {
    out[i] = k_connect[i];
  }
  return (uint32_t)sizeof(k_connect);
}

static void exp29_init(void)
{
  uint8_t pkt[32] = {0};

  U585_Board_InitBasicGpio();
  g_u585_exp29_state.magic = 0xA585001DUL;
  g_u585_exp29_state.iterations = 0U;
  g_u585_exp29_state.tick_ms = HAL_GetTick();
  g_u585_exp29_state.wifi_ready = 0U;      /* demo27 flow_ok=0 */
  g_u585_exp29_state.freertos_ready = 0U;  /* demo28 deferred */
  g_u585_exp29_state.mqtt_ready = 0U;

  U585_Log_WriteLine("");
  U585_Log_WriteLine("[U585][29] MQTT CONNECT frame only (no broker)");
  U585_Log_WriteLine("[U585][29] blocked on WiFi(27) + FreeRTOS(28)");

  g_u585_exp29_state.connect_pkt_len = exp29_build_connect(pkt, sizeof(pkt));
  g_u585_exp29_state.connect_pkt_ok =
      ((g_u585_exp29_state.connect_pkt_len == 14U) && (pkt[0] == 0x10U) && (pkt[1] == 0x0CU)) ? 1U : 0U;

  U585_Log_WriteU32("[U585][29] wifi_ready=", g_u585_exp29_state.wifi_ready);
  U585_Log_WriteU32("[U585][29] freertos_ready=", g_u585_exp29_state.freertos_ready);
  U585_Log_WriteU32("[U585][29] mqtt_ready=", g_u585_exp29_state.mqtt_ready);
  U585_Log_WriteU32("[U585][29] connect_pkt_len=", g_u585_exp29_state.connect_pkt_len);
  U585_Log_WriteU32("[U585][29] connect_pkt_ok=", g_u585_exp29_state.connect_pkt_ok);
}

static void exp29_loop(void)
{
  g_u585_exp29_state.iterations++;
  g_u585_exp29_state.tick_ms = HAL_GetTick();

  if ((g_u585_exp29_state.iterations % 4U) == 0U)
  {
    U585_Log_WriteU32("[U585][29] heartbeat=", g_u585_exp29_state.iterations);
  }

  U585_Board_ToggleGreenLed();
  HAL_Delay(500U);
}

const U585_Demo U585_Demo_Exp29 = {
  "29",
  "MQTT CONNECT frame (offline stub)",
  exp29_init,
  exp29_loop,
};
