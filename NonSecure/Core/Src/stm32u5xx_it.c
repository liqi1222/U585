/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32u5xx_it.c
  * @brief   Interrupt Service Routines.
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32u5xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "u585_adc1.h"
#include "u585_dac1.h"
#include "u585_fault.h"
#include "u585_gpdma.h"
#include "u585_tim2_trgo.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex Processor Interruption and Exception Handlers          */
/******************************************************************************/
static uint32_t *u585_get_active_stack_pointer(void)
{
  uint32_t *stack_ptr;

  __asm volatile(
      "tst lr, #4 \n"
      "ite eq \n"
      "mrseq %0, msp \n"
      "mrsne %0, psp \n"
      : "=r"(stack_ptr));

  return stack_ptr;
}

void HardFault_Handler(void)
{
  U585_Fault_PrintAndHalt("HardFault", u585_get_active_stack_pointer());
}

void BusFault_Handler(void)
{
  U585_Fault_PrintAndHalt("BusFault", u585_get_active_stack_pointer());
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */
  U585_Fault_PrintAndHalt("MemManage", u585_get_active_stack_pointer());
  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */
  U585_Fault_PrintAndHalt("UsageFault", u585_get_active_stack_pointer());
  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32U5xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32u5xx.s).                    */
/******************************************************************************/

/* USER CODE BEGIN 1 */

void U585_Exp09_OnExti(uint16_t pin);
void U585_Exp10_OnTimUpdate(void);

void EXTI13_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
}

void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
  U585_Exp09_OnExti(GPIO_Pin);
}

void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin)
{
  U585_Exp09_OnExti(GPIO_Pin);
}

void TIM2_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&htim2_ns);
}

void ADC1_IRQHandler(void)
{
  HAL_ADC_IRQHandler(&hadc1_ns);
}

void DAC1_IRQHandler(void)
{
  HAL_DAC_IRQHandler(&hdac1_ns);
}

void GPDMA1_Channel1_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_gpdma1_ch1_adc_ns);
}

void GPDMA1_Channel2_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_gpdma1_ch2_dac_ns);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM2)
  {
    U585_Exp10_OnTimUpdate();
  }
}

/* USER CODE END 1 */
