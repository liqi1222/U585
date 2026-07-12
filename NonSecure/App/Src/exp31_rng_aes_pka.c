#include "u585_board.h"
#include "u585_demo.h"
#include "u585_log.h"

typedef struct
{
  uint32_t magic;
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t rng_ok;
  uint32_t aes_ok;
  uint32_t pka_ok;
  uint32_t r0;
  uint32_t r1;
  uint32_t r2;
  uint32_t r3;
  uint32_t aes_ct0;
  uint32_t pka_sum0;
} U585_Exp31State;

volatile U585_Exp31State g_u585_exp31_state;

static RNG_HandleTypeDef hrng_ns;
static CRYP_HandleTypeDef hcryp_ns;
static PKA_HandleTypeDef hpka_ns;

static uint8_t s_aes_key[16] = {
  0x2BU, 0x7EU, 0x15U, 0x16U, 0x28U, 0xAEU, 0xD2U, 0xA6U,
  0xABU, 0xF7U, 0x15U, 0x88U, 0x09U, 0xCFU, 0x4FU, 0x3CU
};
static uint8_t s_aes_pt[16] = {
  0x6BU, 0xC1U, 0xBEU, 0xE2U, 0x2EU, 0x40U, 0x9FU, 0x96U,
  0xE9U, 0x3DU, 0x7EU, 0x11U, 0x73U, 0x93U, 0x17U, 0x2AU
};

static HAL_StatusTypeDef exp31_rng_probe(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  uint32_t i;

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RNG;
  PeriphClkInit.RngClockSelection = RCC_RNGCLKSOURCE_HSI48;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    return HAL_ERROR;
  }

  __HAL_RCC_RNG_CLK_ENABLE();
  hrng_ns.Instance = RNG;
  hrng_ns.Init.ClockErrorDetection = RNG_CED_ENABLE;
  if (HAL_RNG_Init(&hrng_ns) != HAL_OK)
  {
    return HAL_ERROR;
  }

  for (i = 0U; i < 4U; i++)
  {
    uint32_t v = 0U;
    if (HAL_RNG_GenerateRandomNumber(&hrng_ns, &v) != HAL_OK)
    {
      return HAL_ERROR;
    }
    if (i == 0U) { g_u585_exp31_state.r0 = v; }
    else if (i == 1U) { g_u585_exp31_state.r1 = v; }
    else if (i == 2U) { g_u585_exp31_state.r2 = v; }
    else { g_u585_exp31_state.r3 = v; }
  }

  /* Weak sanity: not all identical (extremely unlikely for true RNG). */
  if ((g_u585_exp31_state.r0 == g_u585_exp31_state.r1) &&
      (g_u585_exp31_state.r1 == g_u585_exp31_state.r2) &&
      (g_u585_exp31_state.r2 == g_u585_exp31_state.r3))
  {
    return HAL_ERROR;
  }
  return HAL_OK;
}

static HAL_StatusTypeDef exp31_aes_probe(void)
{
  uint8_t ct[16] = {0};
  uint8_t pt_out[16] = {0};
  uint32_t i;

  __HAL_RCC_AES_CLK_ENABLE();
  hcryp_ns.Instance = AES;
  hcryp_ns.Init.DataType = CRYP_NO_SWAP;
  hcryp_ns.Init.KeySize = CRYP_KEYSIZE_128B;
  hcryp_ns.Init.pKey = (uint32_t *)(void *)s_aes_key;
  hcryp_ns.Init.Algorithm = CRYP_AES_ECB;
  hcryp_ns.Init.DataWidthUnit = CRYP_DATAWIDTHUNIT_BYTE;
  hcryp_ns.Init.HeaderWidthUnit = CRYP_HEADERWIDTHUNIT_BYTE;
  hcryp_ns.Init.KeyIVConfigSkip = CRYP_KEYIVCONFIG_ALWAYS;
  hcryp_ns.Init.KeyMode = CRYP_KEYMODE_NORMAL;

  if (HAL_CRYP_Init(&hcryp_ns) != HAL_OK)
  {
    return HAL_ERROR;
  }

  if (HAL_CRYP_Encrypt(&hcryp_ns, (uint32_t *)(void *)s_aes_pt, 16U,
                       (uint32_t *)(void *)ct, 100U) != HAL_OK)
  {
    return HAL_ERROR;
  }

  g_u585_exp31_state.aes_ct0 =
      ((uint32_t)ct[0] << 24) | ((uint32_t)ct[1] << 16) |
      ((uint32_t)ct[2] << 8) | (uint32_t)ct[3];

  if (HAL_CRYP_Decrypt(&hcryp_ns, (uint32_t *)(void *)ct, 16U,
                       (uint32_t *)(void *)pt_out, 100U) != HAL_OK)
  {
    return HAL_ERROR;
  }

  for (i = 0U; i < 16U; i++)
  {
    if (pt_out[i] != s_aes_pt[i])
    {
      return HAL_ERROR;
    }
  }
  return HAL_OK;
}

static HAL_StatusTypeDef exp31_pka_probe(void)
{
  /* Tiny modular add: (5 + 7) mod 11 = 1. */
  uint32_t opA[1] = {5U};
  uint32_t opB[1] = {7U};
  /* pOp3 is size*4 bytes in big-endian (HAL reverses into PKA RAM). */
  uint8_t mod[4] = {0U, 0U, 0U, 11U};
  uint32_t result[2] = {0U, 0U};
  PKA_ModAddInTypeDef in;

  __HAL_RCC_PKA_CLK_ENABLE();
  hpka_ns.Instance = PKA;
  if (HAL_PKA_Init(&hpka_ns) != HAL_OK)
  {
    return HAL_ERROR;
  }

  in.size = 1U;
  in.pOp1 = opA;
  in.pOp2 = opB;
  in.pOp3 = mod;

  if (HAL_PKA_ModAdd(&hpka_ns, &in, 1000U) != HAL_OK)
  {
    return HAL_ERROR;
  }

  HAL_PKA_Arithmetic_GetResult(&hpka_ns, result);
  g_u585_exp31_state.pka_sum0 = result[0];
  return (result[0] == 1U) ? HAL_OK : HAL_ERROR;
}

static void exp31_init(void)
{
  U585_Board_InitBasicGpio();
  g_u585_exp31_state.magic = 0xA585001FUL;
  g_u585_exp31_state.iterations = 0U;
  g_u585_exp31_state.tick_ms = HAL_GetTick();

  U585_Log_WriteLine("");
  U585_Log_WriteLine("[U585][31] RNG + AES-ECB + PKA ModAdd probe");
  U585_Log_WriteLine("[U585][31] HASH/SAES/TLS pending");

  g_u585_exp31_state.rng_ok = (exp31_rng_probe() == HAL_OK) ? 1U : 0U;
  U585_Log_WriteU32("[U585][31] rng_ok=", g_u585_exp31_state.rng_ok);
  U585_Log_WriteU32("[U585][31] r0=", g_u585_exp31_state.r0);
  U585_Log_WriteU32("[U585][31] r1=", g_u585_exp31_state.r1);
  U585_Log_WriteU32("[U585][31] r2=", g_u585_exp31_state.r2);
  U585_Log_WriteU32("[U585][31] r3=", g_u585_exp31_state.r3);

  g_u585_exp31_state.aes_ok = (exp31_aes_probe() == HAL_OK) ? 1U : 0U;
  U585_Log_WriteU32("[U585][31] aes_ok=", g_u585_exp31_state.aes_ok);
  U585_Log_WriteU32("[U585][31] aes_ct0=", g_u585_exp31_state.aes_ct0);

  g_u585_exp31_state.pka_ok = (exp31_pka_probe() == HAL_OK) ? 1U : 0U;
  U585_Log_WriteU32("[U585][31] pka_ok=", g_u585_exp31_state.pka_ok);
  U585_Log_WriteU32("[U585][31] pka_sum0=", g_u585_exp31_state.pka_sum0);
}

static void exp31_loop(void)
{
  g_u585_exp31_state.iterations++;
  g_u585_exp31_state.tick_ms = HAL_GetTick();

  if ((g_u585_exp31_state.iterations % 4U) == 0U)
  {
    U585_Log_WriteU32("[U585][31] heartbeat=", g_u585_exp31_state.iterations);
  }

  U585_Board_ToggleGreenLed();
  HAL_Delay(500U);
}

const U585_Demo U585_Demo_Exp31 = {
  "31",
  "RNG AES PKA hardware crypto probe",
  exp31_init,
  exp31_loop,
};
