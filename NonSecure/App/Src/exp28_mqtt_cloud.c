#include "u585_board.h"
#include "u585_demo.h"
#include "u585_log.h"

typedef struct
{
  uint32_t magic;
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t wifi_ready;
  uint32_t mqtt_ready;
  uint32_t connect_pkt_len;
  uint32_t connect_pkt_ok;
} U585_Exp28State;

volatile U585_Exp28State g_u585_exp28_state;

/*
 * Minimal MQTT CONNECT (v3.1.1) framing without a network stack.
 * Fixed header + remaining length + variable header + empty client-id payload
 * is enough to prove packet layout knowledge before Wi-Fi lands.
 */
static uint32_t exp28_build_connect(uint8_t *out, uint32_t out_cap)
{
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

static void exp28_init(void)
{
  uint8_t pkt[32] = {0};

  U585_Board_InitBasicGpio();
  g_u585_exp28_state.magic = 0xA585001CUL;
  g_u585_exp28_state.iterations = 0U;
  g_u585_exp28_state.tick_ms = HAL_GetTick();
  g_u585_exp28_state.wifi_ready = 0U; /* demo27 flow_ok=0 */
  g_u585_exp28_state.mqtt_ready = 0U;

  U585_Log_WriteLine("");
  U585_Log_WriteLine("[U585][28] MQTT CONNECT frame only (no broker)");
  U585_Log_WriteLine("[U585][28] blocked on WiFi(27); RTOS out of series scope");

  g_u585_exp28_state.connect_pkt_len = exp28_build_connect(pkt, sizeof(pkt));
  g_u585_exp28_state.connect_pkt_ok =
      ((g_u585_exp28_state.connect_pkt_len == 14U) && (pkt[0] == 0x10U) && (pkt[1] == 0x0CU)) ? 1U : 0U;

  U585_Log_WriteU32("[U585][28] wifi_ready=", g_u585_exp28_state.wifi_ready);
  U585_Log_WriteU32("[U585][28] mqtt_ready=", g_u585_exp28_state.mqtt_ready);
  U585_Log_WriteU32("[U585][28] connect_pkt_len=", g_u585_exp28_state.connect_pkt_len);
  U585_Log_WriteU32("[U585][28] connect_pkt_ok=", g_u585_exp28_state.connect_pkt_ok);
}

static void exp28_loop(void)
{
  g_u585_exp28_state.iterations++;
  g_u585_exp28_state.tick_ms = HAL_GetTick();

  if ((g_u585_exp28_state.iterations % 4U) == 0U)
  {
    U585_Log_WriteU32("[U585][28] heartbeat=", g_u585_exp28_state.iterations);
  }

  U585_Board_ToggleGreenLed();
  HAL_Delay(500U);
}

const U585_Demo U585_Demo_Exp28 = {
  "28",
  "MQTT CONNECT frame (offline stub)",
  exp28_init,
  exp28_loop,
};
