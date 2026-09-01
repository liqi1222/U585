/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    Secure/Src/secure_nsc.c
  * @author  MCD Application Team
  * @brief   This file contains the non-secure callable APIs (secure world)
  ******************************************************************************
    * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* USER CODE BEGIN Non_Secure_CallLib */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "secure_nsc.h"
#include "usart.h"
/** @addtogroup STM32U5xx_HAL_Examples

  * @{
  */

/** @addtogroup Templates
  * @{
  */

/* Global variables ----------------------------------------------------------*/
void *pSecureFaultCallback = NULL;   /* Pointer to secure fault callback in Non-secure */
void *pSecureErrorCallback = NULL;   /* Pointer to secure error callback in Non-secure */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Secure registration of non-secure callback.
  * @param  CallbackId  callback identifier
  * @param  func        pointer to non-secure function
  * @retval None
  */
CMSE_NS_ENTRY void SECURE_RegisterCallback(SECURE_CallbackIDTypeDef CallbackId, void *func)
{
  if(func != NULL)
  {
    switch(CallbackId)
    {
      case SECURE_FAULT_CB_ID:           /* SecureFault Interrupt occurred */
        pSecureFaultCallback = func;
        break;
      case GTZC_ERROR_CB_ID:             /* GTZC Interrupt occurred */
        pSecureErrorCallback = func;
        break;
      default:
        /* unknown */
        break;
    }
  }
}

CMSE_NS_ENTRY void SECURE_UART1_WriteString(const char *text)
{
  const char *cursor = text;
  uint16_t length = 0U;

  if (cursor == NULL)
  {
    return;
  }

  while ((cursor[length] != '\0') && (length < 256U))
  {
    length++;
  }

  if (length > 0U)
  {
    (void)HAL_UART_Transmit(&huart1, (uint8_t *)text, length, 100U);
  }
}

CMSE_NS_ENTRY uint32_t SECURE_GetTzMagic(void)
{
  return 0xA5850032UL;
}

CMSE_NS_ENTRY uint32_t SECURE_GetSauRegionCount(void)
{
  return (SAU->TYPE & SAU_TYPE_SREGION_Msk) >> SAU_TYPE_SREGION_Pos;
}

CMSE_NS_ENTRY uint32_t SECURE_GetFlashSecureBase(void)
{
  return 0x0C000000UL;
}

CMSE_NS_ENTRY uint32_t SECURE_GetFlashNonSecureBase(void)
{
  return 0x08100000UL;
}

/* ------------------------------------------------------------------------- */
/* OTFDEC demo API for exp33 (OSPI PSRAM + Flash on-the-fly decryption test) */
/* Keys never leave the secure world; NonSecure only sees the public key CRC */
/* and the decrypted/ciphertext data through the OCTOSPI memory mapping.     */
/* ------------------------------------------------------------------------- */

#define U585_OTFDEC_SEL_PSRAM       1U  /* OTFDEC1 <-> OCTOSPI1 @0x90000000 */
#define U585_OTFDEC_SEL_FLASH       2U  /* OTFDEC2 <-> OCTOSPI2 @0x70000000 */
#define U585_OTFDEC_PSRAM_START     0x90000000UL
#define U585_OTFDEC_PSRAM_END       0x90000FFFUL
#define U585_OTFDEC_FLASH_START     0x70000000UL
#define U585_OTFDEC_FLASH_END       0x70000FFFUL
#define U585_OTFDEC_DEMO_VERSION    0x0007U
#define U585_OTFDEC_CIPHER_WORDS    16U  /* fixed 64-byte test vector (NSC entries are */
                                         /* limited to 4 register-passed arguments)    */

static OTFDEC_HandleTypeDef hotfdec1_s;
static OTFDEC_HandleTypeDef hotfdec2_s;
static uint8_t otfdec_demo_ready = 0U;

/* Demo-only key/nonce (AES-128 CTR). Real products must provision these     */
/* through a trusted channel and lock the region/key after programming.      */
static uint32_t otfdec_demo_key[4] = {0x2B7E1516UL, 0x28AED2A6UL, 0xABF71588UL, 0x09CF4F3CUL};
static const uint32_t otfdec_demo_nonce[2] = {0x5538354FUL, 0x53454433UL};

static uint32_t SECURE_OTFDEC_SetupInstance(OTFDEC_HandleTypeDef *hotfdec, OTFDEC_TypeDef *instance,
                                            uint32_t start_addr, uint32_t end_addr)
{
  OTFDEC_RegionConfigTypeDef config;

  hotfdec->Instance = instance;
  if (HAL_OTFDEC_Init(hotfdec) != HAL_OK)
  {
    return 1U;
  }
  /* MODE must be programmed before the key: changing MODE clears the key. */
  if (HAL_OTFDEC_RegionSetMode(hotfdec, OTFDEC_REGION1,
                               OTFDEC_REG_MODE_INSTRUCTION_OR_DATA_ACCESSES) != HAL_OK)
  {
    return 2U;
  }
  if (HAL_OTFDEC_RegionSetKey(hotfdec, OTFDEC_REGION1, otfdec_demo_key) != HAL_OK)
  {
    return 3U;
  }
  /* Public integrity check of the just-written secret key. */
  if ((HAL_OTFDEC_RegionGetKeyCRC(hotfdec, OTFDEC_REGION1) & 0xFFU) !=
      (HAL_OTFDEC_KeyCRCComputation(otfdec_demo_key) & 0xFFU))
  {
    return 4U;
  }
  config.Nonce[0]     = otfdec_demo_nonce[0];
  config.Nonce[1]     = otfdec_demo_nonce[1];
  config.StartAddress = start_addr;
  config.EndAddress   = end_addr;
  config.Version      = U585_OTFDEC_DEMO_VERSION;
  /* Keep the configuration unlocked during development so the demo can     */
  /* toggle REG_EN to demonstrate ciphertext vs plaintext reads.            */
  if (HAL_OTFDEC_RegionConfig(hotfdec, OTFDEC_REGION1, &config,
                              OTFDEC_REG_CONFIGR_LOCK_DISABLE) != HAL_OK)
  {
    return 5U;
  }
  if (HAL_OTFDEC_RegionEnable(hotfdec, OTFDEC_REGION1) != HAL_OK)
  {
    return 6U;
  }
  return 0U;
}

static OTFDEC_HandleTypeDef *SECURE_OTFDEC_Select(uint32_t instance_sel, uint32_t *region_base)
{
  if (instance_sel == U585_OTFDEC_SEL_PSRAM)
  {
    *region_base = U585_OTFDEC_PSRAM_START;
    return &hotfdec1_s;
  }
  if (instance_sel == U585_OTFDEC_SEL_FLASH)
  {
    *region_base = U585_OTFDEC_FLASH_START;
    return &hotfdec2_s;
  }
  return NULL;
}

/**
  * @brief  Program OTFDEC1 (PSRAM region @0x90000000) and OTFDEC2
  *         (flash region @0x70000000), one 4 KByte region each, and enable
  *         on-the-fly decryption on both.
  *         Also opens the GTZC MPCWM watermarks: without this the OCTOSPI
  *         memory apertures stay secure-only after reset, so every
  *         NonSecure memory-mapped read returns 0 and writes are dropped.
  * @retval Packed status:
  *         bits[3:0]   = OTFDEC1 setup error code (0 = OK)
  *         bits[7:4]   = OTFDEC2 setup error code (0 = OK)
  *         bits[15:8]  = OTFDEC1 region1 key CRC
  *         bits[23:16] = OTFDEC2 region1 key CRC
  *         bit[27]     = 1 when the OCTOSPI1 MPCWM open failed
  *         bit[28]     = 1 when the OCTOSPI2 MPCWM open failed
  *         bit[31]     = 1 when both instances are ready
  */
CMSE_NS_ENTRY uint32_t SECURE_OTFDEC_Setup(void)
{
  uint32_t err1;
  uint32_t err2;
  uint32_t wm1_fail = 0U;
  uint32_t wm2_fail = 0U;
  MPCWM_ConfigTypeDef mpcwm;

  __HAL_RCC_OTFDEC1_CLK_ENABLE();
  __HAL_RCC_OTFDEC2_CLK_ENABLE();

  /* Open both OCTOSPI apertures to NonSecure access (whole 256 MB each,  */
  /* 128 KB granularity, non-secure + non-privileged = attribute 0).      */
  mpcwm.AreaId     = GTZC_TZSC_MPCWM_ID1;
  mpcwm.Offset     = 0U;
  mpcwm.Length     = 0x10000000UL;
  mpcwm.Attribute  = GTZC_TZSC_MPCWM_REGION_NSEC | GTZC_TZSC_MPCWM_REGION_NPRIV;
  mpcwm.Lock       = GTZC_TZSC_MPCWM_LOCK_OFF;
  mpcwm.AreaStatus = ENABLE;
  if (HAL_GTZC_TZSC_MPCWM_ConfigMemAttributes(OCTOSPI1_BASE, &mpcwm) != HAL_OK)
  {
    wm1_fail = 1U;
  }
  if (HAL_GTZC_TZSC_MPCWM_ConfigMemAttributes(OCTOSPI2_BASE, &mpcwm) != HAL_OK)
  {
    wm2_fail = 1U;
  }

  err1 = SECURE_OTFDEC_SetupInstance(&hotfdec1_s, OTFDEC1,
                                     U585_OTFDEC_PSRAM_START, U585_OTFDEC_PSRAM_END);
  err2 = SECURE_OTFDEC_SetupInstance(&hotfdec2_s, OTFDEC2,
                                     U585_OTFDEC_FLASH_START, U585_OTFDEC_FLASH_END);
  otfdec_demo_ready = ((err1 == 0U) && (err2 == 0U)) ? 1U : 0U;

  return (err1 & 0xFU) | ((err2 & 0xFU) << 4) |
         ((HAL_OTFDEC_RegionGetKeyCRC(&hotfdec1_s, OTFDEC_REGION1) & 0xFFU) << 8) |
         ((HAL_OTFDEC_RegionGetKeyCRC(&hotfdec2_s, OTFDEC_REGION1) & 0xFFU) << 16) |
         (wm1_fail << 27) | (wm2_fail << 28) |
         ((uint32_t)otfdec_demo_ready << 31);
}

/**
  * @brief  Enable/disable decryption on one OTFDEC demo region.
  * @param  instance_sel  1 = OTFDEC1 (PSRAM), 2 = OTFDEC2 (flash)
  * @param  enable        0 = disable (memory-mapped reads return raw data),
  *                       nonzero = enable (reads return plaintext)
  * @retval 0 = OK, 2 = bad parameter/state, 1 = HAL error
  */
CMSE_NS_ENTRY uint32_t SECURE_OTFDEC_RegionEnable(uint32_t instance_sel, uint32_t enable)
{
  uint32_t region_base;
  OTFDEC_HandleTypeDef *hotfdec = SECURE_OTFDEC_Select(instance_sel, &region_base);

  (void)region_base;
  if ((hotfdec == NULL) || (otfdec_demo_ready == 0U))
  {
    return 2U;
  }
  if (enable != 0U)
  {
    return (HAL_OTFDEC_RegionEnable(hotfdec, OTFDEC_REGION1) == HAL_OK) ? 0U : 1U;
  }
  return (HAL_OTFDEC_RegionDisable(hotfdec, OTFDEC_REGION1) == HAL_OK) ? 0U : 1U;
}

/**
  * @brief  Encipher NS plaintext with the region key/nonce/version using the
  *         OTFDEC encryption mode (ENC bit). The ciphertext is returned to
  *         NonSecure so it can be stored into the external memory; later
  *         memory-mapped reads with the region enabled return the plaintext.
  *         The test vector length is fixed at 16 words (64 bytes).
  * @param  instance_sel   1 = OTFDEC1 (PSRAM), 2 = OTFDEC2 (flash)
  * @param  start_address  target address inside the demo region
  * @param  input          NS plaintext buffer (16 x 32-bit words)
  * @param  output         NS ciphertext buffer (16 x 32-bit words)
  * @retval 0 = OK, 2 = bad parameter/state, 3 = NS pointer check failed,
  *         4 = enciphering enable failed, 1 = HAL error
  */
CMSE_NS_ENTRY uint32_t SECURE_OTFDEC_Cipher(uint32_t instance_sel, uint32_t start_address,
                                            const uint32_t *input, uint32_t *output)
{
  uint32_t region_base;
  OTFDEC_HandleTypeDef *hotfdec = SECURE_OTFDEC_Select(instance_sel, &region_base);
  HAL_StatusTypeDef status;

  if ((hotfdec == NULL) || (otfdec_demo_ready == 0U) ||
      (input == NULL) || (output == NULL))
  {
    return 2U;
  }
  if ((start_address < region_base) ||
      (start_address > (region_base + 0x1000U - (U585_OTFDEC_CIPHER_WORDS * 4U))))
  {
    return 2U;
  }
  if (cmse_check_address_range((void *)input, U585_OTFDEC_CIPHER_WORDS * 4U,
                               CMSE_NONSECURE | CMSE_MPU_READ) == NULL)
  {
    return 3U;
  }
  if (cmse_check_address_range((void *)output, U585_OTFDEC_CIPHER_WORDS * 4U,
                               CMSE_NONSECURE | CMSE_MPU_READWRITE) == NULL)
  {
    return 3U;
  }
  /* The OTFDEC encryption mode only takes effect when ENC is set while the  */
  /* region is disabled (HAL doc orders enciphering enable before the region */
  /* setup): disable region -> set ENC -> re-enable -> cipher -> restore.    */
  /* While ENC is set the OTFDEC intercepts the AHB traffic: plaintext is    */
  /* written to the region address and read back encrypted.                  */
  if (HAL_OTFDEC_RegionDisable(hotfdec, OTFDEC_REGION1) != HAL_OK)
  {
    return 4U;
  }
  if (HAL_OTFDEC_EnableEnciphering(hotfdec) != HAL_OK)
  {
    return 4U;
  }
  if ((hotfdec->Instance->CR & OTFDEC_CR_ENC) == 0U)
  {
    (void)HAL_OTFDEC_RegionEnable(hotfdec, OTFDEC_REGION1);
    return 5U;  /* ENC bit did not stick */
  }
  if (HAL_OTFDEC_RegionEnable(hotfdec, OTFDEC_REGION1) != HAL_OK)
  {
    (void)HAL_OTFDEC_DisableEnciphering(hotfdec);
    return 4U;
  }
  status = HAL_OTFDEC_Cipher(hotfdec, OTFDEC_REGION1, input, output,
                             U585_OTFDEC_CIPHER_WORDS, start_address);
  (void)HAL_OTFDEC_RegionDisable(hotfdec, OTFDEC_REGION1);
  (void)HAL_OTFDEC_DisableEnciphering(hotfdec);
  (void)HAL_OTFDEC_RegionEnable(hotfdec, OTFDEC_REGION1);
  return (status == HAL_OK) ? 0U : 1U;
}

/**
  * @brief  Encipher NS plaintext in software with the AES (CRYP) peripheral,
  *         replicating exactly the OTFDEC AES-128-CTR keystream of the demo
  *         region (same key/nonce/version/address-derived IV), following the
  *         official STM32CubeU5 OTFDEC_Data_Decrypt example:
  *           - CRYP key words are swapped (K3,K2,K1,K0)
  *           - IV[3] = (region%4)<<28 | (start_address>>4)
  *           - IV[2] = version, IV[1] = Nonce[0], IV[0] = Nonce[1]
  *           - each 4-word block is reversed before/after encryption
  *         Use this when the ciphertext must be stored into the NOR flash:
  *         memory-mapped writes cannot page-program the MX25LM, so the NS
  *         writes the returned ciphertext with indirect page-program commands.
  * @param  instance_sel   1 = OTFDEC1 (PSRAM), 2 = OTFDEC2 (flash)
  * @param  start_address  target address inside the demo region
  * @param  input          NS plaintext buffer (16 x 32-bit words)
  * @param  output         NS ciphertext buffer (16 x 32-bit words)
  * @retval 0 = OK, 2 = bad parameter/state, 3 = NS pointer check failed,
  *         6 = CRYP init failed, 1 = CRYP encrypt failed
  */
CMSE_NS_ENTRY uint32_t SECURE_OTFDEC_CipherSw(uint32_t instance_sel, uint32_t start_address,
                                              const uint32_t *input, uint32_t *output)
{
  uint32_t region_base;
  OTFDEC_HandleTypeDef *hotfdec = SECURE_OTFDEC_Select(instance_sel, &region_base);
  CRYP_HandleTypeDef hcryp;
  uint32_t key_swapped[4];
  uint32_t iv[4];
  uint32_t swapped_in[U585_OTFDEC_CIPHER_WORDS];
  uint32_t j;
  HAL_StatusTypeDef status;

  if ((hotfdec == NULL) || (otfdec_demo_ready == 0U) ||
      (input == NULL) || (output == NULL))
  {
    return 2U;
  }
  if ((start_address < region_base) ||
      (start_address > (region_base + 0x1000U - (U585_OTFDEC_CIPHER_WORDS * 4U))))
  {
    return 2U;
  }
  if (cmse_check_address_range((void *)input, U585_OTFDEC_CIPHER_WORDS * 4U,
                               CMSE_NONSECURE | CMSE_MPU_READ) == NULL)
  {
    return 3U;
  }
  if (cmse_check_address_range((void *)output, U585_OTFDEC_CIPHER_WORDS * 4U,
                               CMSE_NONSECURE | CMSE_MPU_READWRITE) == NULL)
  {
    return 3U;
  }

  key_swapped[0] = otfdec_demo_key[3];
  key_swapped[1] = otfdec_demo_key[2];
  key_swapped[2] = otfdec_demo_key[1];
  key_swapped[3] = otfdec_demo_key[0];

  iv[3] = ((OTFDEC_REGION1 % 4U) << 28) | (start_address >> 4);
  iv[2] = U585_OTFDEC_DEMO_VERSION;
  iv[1] = otfdec_demo_nonce[0];
  iv[0] = otfdec_demo_nonce[1];

  for (j = 0U; j < (U585_OTFDEC_CIPHER_WORDS / 4U); j++)
  {
    swapped_in[4U * j]       = input[(4U * j) + 3U];
    swapped_in[(4U * j) + 1U] = input[(4U * j) + 2U];
    swapped_in[(4U * j) + 2U] = input[(4U * j) + 1U];
    swapped_in[(4U * j) + 3U] = input[4U * j];
  }

  __HAL_RCC_AES_CLK_ENABLE();
  hcryp.Instance = AES;
  hcryp.Init.DataType         = CRYP_NO_SWAP;
  hcryp.Init.KeySize          = CRYP_KEYSIZE_128B;
  hcryp.Init.pKey             = key_swapped;
  hcryp.Init.pInitVect        = iv;
  hcryp.Init.Algorithm        = CRYP_AES_CTR;
  hcryp.Init.DataWidthUnit    = CRYP_DATAWIDTHUNIT_WORD;
  hcryp.Init.HeaderWidthUnit  = CRYP_HEADERWIDTHUNIT_WORD;
  hcryp.Init.KeyIVConfigSkip  = CRYP_KEYIVCONFIG_ALWAYS;
  hcryp.Init.KeyMode          = CRYP_KEYMODE_NORMAL;
  if (HAL_CRYP_Init(&hcryp) != HAL_OK)
  {
    return 6U;
  }
  status = HAL_CRYP_Encrypt(&hcryp, swapped_in, U585_OTFDEC_CIPHER_WORDS,
                            output, 0x1000U);
  (void)HAL_CRYP_DeInit(&hcryp);
  if (status != HAL_OK)
  {
    return 1U;
  }

  for (j = 0U; j < (U585_OTFDEC_CIPHER_WORDS / 4U); j++)
  {
    uint32_t tmp            = output[4U * j];
    output[4U * j]          = output[(4U * j) + 3U];
    output[(4U * j) + 3U]   = tmp;
    tmp                     = output[(4U * j) + 1U];
    output[(4U * j) + 1U]   = output[(4U * j) + 2U];
    output[(4U * j) + 2U]   = tmp;
  }
  return 0U;
}

/**
  * @brief  Read back one OTFDEC register for diagnosis.
  * @param  instance_sel  1 = OTFDEC1 (PSRAM), 2 = OTFDEC2 (flash)
  * @param  which         0 = OTFDEC_CR, 1 = region1 OTFDEC_R1CFGR
  * @retval register value, or 0xFFFFFFFF on bad parameter
  */
CMSE_NS_ENTRY uint32_t SECURE_OTFDEC_GetReg(uint32_t instance_sel, uint32_t which)
{
  uint32_t region_base;
  OTFDEC_HandleTypeDef *hotfdec = SECURE_OTFDEC_Select(instance_sel, &region_base);

  (void)region_base;
  if ((hotfdec == NULL) || (otfdec_demo_ready == 0U))
  {
    return 0xFFFFFFFFUL;
  }
  if (which == 0U)
  {
    return hotfdec->Instance->CR;
  }
  if (which == 1U)
  {
    const OTFDEC_Region_TypeDef *region =
        (OTFDEC_Region_TypeDef *)((uint32_t)hotfdec->Instance + 0x20U +
                                  (0x30U * OTFDEC_REGION1));
    return region->REG_CONFIGR;
  }
  return 0xFFFFFFFFUL;
}

/**
  * @brief  Read the public 8-bit key CRC of one demo region.
  * @param  instance_sel  1 = OTFDEC1 (PSRAM), 2 = OTFDEC2 (flash)
  * @retval key CRC in bits[7:0], or 0xFFFFFFFF on bad parameter
  */
CMSE_NS_ENTRY uint32_t SECURE_OTFDEC_GetKeyCrc(uint32_t instance_sel)
{
  uint32_t region_base;
  OTFDEC_HandleTypeDef *hotfdec = SECURE_OTFDEC_Select(instance_sel, &region_base);

  (void)region_base;
  if ((hotfdec == NULL) || (otfdec_demo_ready == 0U))
  {
    return 0xFFFFFFFFUL;
  }
  return HAL_OTFDEC_RegionGetKeyCRC(hotfdec, OTFDEC_REGION1) & 0xFFU;
}

/**
  * @}
  */

/**
  * @}
  */
/* USER CODE END Non_Secure_CallLib */

