/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    Secure_nsclib/secure_nsc.h
  * @author  MCD Application Team
  * @brief   Header for secure non-secure callable APIs list
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

/* USER CODE BEGIN Non_Secure_CallLib_h */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef SECURE_NSC_H
#define SECURE_NSC_H

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Exported types ------------------------------------------------------------*/
/**
  * @brief  non-secure callback ID enumeration definition
  */
typedef enum
{
  SECURE_FAULT_CB_ID     = 0x00U, /*!< System secure fault callback ID */
  GTZC_ERROR_CB_ID       = 0x01U  /*!< GTZC secure error callback ID */
} SECURE_CallbackIDTypeDef;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void SECURE_RegisterCallback(SECURE_CallbackIDTypeDef CallbackId, void *func);
void SECURE_UART1_WriteString(const char *text);
uint32_t SECURE_GetTzMagic(void);
uint32_t SECURE_GetSauRegionCount(void);
uint32_t SECURE_GetFlashSecureBase(void);
uint32_t SECURE_GetFlashNonSecureBase(void);

/* OTFDEC demo API (exp33): keys stay in secure world */
uint32_t SECURE_OTFDEC_Setup(void);
uint32_t SECURE_OTFDEC_RegionEnable(uint32_t instance_sel, uint32_t enable);
uint32_t SECURE_OTFDEC_Cipher(uint32_t instance_sel, uint32_t start_address,
                              const uint32_t *input, uint32_t *output);
uint32_t SECURE_OTFDEC_CipherSw(uint32_t instance_sel, uint32_t start_address,
                                const uint32_t *input, uint32_t *output);
uint32_t SECURE_OTFDEC_GetReg(uint32_t instance_sel, uint32_t which);
uint32_t SECURE_OTFDEC_GetKeyCrc(uint32_t instance_sel);

#endif /* SECURE_NSC_H */
/* USER CODE END Non_Secure_CallLib_h */

