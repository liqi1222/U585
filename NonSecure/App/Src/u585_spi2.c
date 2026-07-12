#include "u585_spi2.h"

SPI_HandleTypeDef hspi2_ns;
static uint8_t u585_spi2_ready = 0U;

void HAL_SPI_MspInit(SPI_HandleTypeDef *spiHandle)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  if (spiHandle->Instance != SPI2)
  {
    return;
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_SPI2;
  PeriphClkInit.Spi2ClockSelection = RCC_SPI2CLKSOURCE_SYSCLK;
  (void)HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_SPI2_CLK_ENABLE();

  /* PD1=SCK, PD3=MISO, PD4=MOSI */
  GPIO_InitStruct.Pin = U585_SPI2_SCK_Pin | U585_SPI2_MISO_Pin | U585_SPI2_MOSI_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* PB12 software NSS (idle high) */
  GPIO_InitStruct.Pin = U585_SPI2_NSS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = 0;
  HAL_GPIO_Init(U585_SPI2_NSS_GPIO_Port, &GPIO_InitStruct);
  HAL_GPIO_WritePin(U585_SPI2_NSS_GPIO_Port, U585_SPI2_NSS_Pin, GPIO_PIN_SET);
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef *spiHandle)
{
  if (spiHandle->Instance != SPI2)
  {
    return;
  }

  __HAL_RCC_SPI2_CLK_DISABLE();
  HAL_GPIO_DeInit(GPIOD, U585_SPI2_SCK_Pin | U585_SPI2_MISO_Pin | U585_SPI2_MOSI_Pin);
  HAL_GPIO_DeInit(U585_SPI2_NSS_GPIO_Port, U585_SPI2_NSS_Pin);
}

HAL_StatusTypeDef U585_SPI2_Init(void)
{
  u585_spi2_ready = 0U;

  hspi2_ns.Instance = SPI2;
  hspi2_ns.Init.Mode = SPI_MODE_MASTER;
  hspi2_ns.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2_ns.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2_ns.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2_ns.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2_ns.Init.NSS = SPI_NSS_SOFT;
  hspi2_ns.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
  hspi2_ns.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2_ns.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2_ns.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2_ns.Init.CRCPolynomial = 0x7;
  hspi2_ns.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  hspi2_ns.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
  hspi2_ns.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
  hspi2_ns.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
  hspi2_ns.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
  hspi2_ns.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
  hspi2_ns.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
  hspi2_ns.Init.IOSwap = SPI_IO_SWAP_DISABLE;
  hspi2_ns.Init.ReadyMasterManagement = SPI_RDY_MASTER_MANAGEMENT_INTERNALLY;
  hspi2_ns.Init.ReadyPolarity = SPI_RDY_POLARITY_HIGH;

  if (HAL_SPI_Init(&hspi2_ns) != HAL_OK)
  {
    return HAL_ERROR;
  }

  u585_spi2_ready = 1U;
  return HAL_OK;
}

uint8_t U585_SPI2_IsReady(void)
{
  return u585_spi2_ready;
}

void U585_SPI2_Nss(uint8_t select)
{
  HAL_GPIO_WritePin(U585_SPI2_NSS_GPIO_Port,
                    U585_SPI2_NSS_Pin,
                    (select != 0U) ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

HAL_StatusTypeDef U585_SPI2_Transfer(uint8_t *tx, uint8_t *rx, uint16_t len, uint32_t timeout_ms)
{
  HAL_StatusTypeDef st;

  if ((u585_spi2_ready == 0U) || (tx == NULL) || (rx == NULL) || (len == 0U))
  {
    return HAL_ERROR;
  }

  U585_SPI2_Nss(1U);
  st = HAL_SPI_TransmitReceive(&hspi2_ns, tx, rx, len, timeout_ms);
  U585_SPI2_Nss(0U);
  return st;
}
