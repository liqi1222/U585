#include "u585_ospi.h"

OSPI_HandleTypeDef hospi_flash_ns;
OSPI_HandleTypeDef hospi_psram_ns;

static uint8_t u585_ospi_flash_ready = 0U;
static uint8_t u585_ospi_psram_ready = 0U;

void HAL_OSPI_MspInit(OSPI_HandleTypeDef *ospiHandle)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_OSPI;
  PeriphClkInit.OspiClockSelection = RCC_OSPICLKSOURCE_SYSCLK;
  (void)HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

  __HAL_RCC_OSPIM_CLK_ENABLE();

  if (ospiHandle->Instance == OCTOSPI2)
  {
    __HAL_RCC_OSPI2_CLK_ENABLE();
    __HAL_RCC_GPIOI_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();

    /* Flash on OCTOSPIM Port2. */
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_OCTOSPI2;
    HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;
    GPIO_InitStruct.Alternate = GPIO_AF5_OCTOSPI2;
    HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_12;
    GPIO_InitStruct.Alternate = GPIO_AF5_OCTOSPI2;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
  }
  else if (ospiHandle->Instance == OCTOSPI1)
  {
    __HAL_RCC_OSPI1_CLK_ENABLE();
    __HAL_RCC_GPIOI_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* PSRAM on OCTOSPIM Port1. */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF3_OCTOSPI1;
    HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Alternate = GPIO_AF3_OCTOSPI1;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_7;
    GPIO_InitStruct.Alternate = GPIO_AF10_OCTOSPI1;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9;
    GPIO_InitStruct.Alternate = GPIO_AF10_OCTOSPI1;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Alternate = GPIO_AF3_OCTOSPI1;
    HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Alternate = GPIO_AF10_OCTOSPI1;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11;
    GPIO_InitStruct.Alternate = GPIO_AF10_OCTOSPI1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  }
}

void HAL_OSPI_MspDeInit(OSPI_HandleTypeDef *ospiHandle)
{
  if (ospiHandle->Instance == OCTOSPI2)
  {
    __HAL_RCC_OSPI2_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOI, GPIO_PIN_5);
    HAL_GPIO_DeInit(GPIOH, GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12);
    HAL_GPIO_DeInit(GPIOF, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_12);
  }
  else if (ospiHandle->Instance == OCTOSPI1)
  {
    __HAL_RCC_OSPI1_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOI, GPIO_PIN_0);
    HAL_GPIO_DeInit(GPIOE, GPIO_PIN_3);
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_7);
    HAL_GPIO_DeInit(GPIOF, GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9);
    HAL_GPIO_DeInit(GPIOH, GPIO_PIN_2);
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_3);
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_10 | GPIO_PIN_11);
  }
}

HAL_StatusTypeDef U585_OSPI_Flash_Init(void)
{
  OSPIM_CfgTypeDef sOspiManagerCfg = {0};
  HAL_OSPI_DLYB_CfgTypeDef dlyb = {0};

  u585_ospi_flash_ready = 0U;

  hospi_flash_ns.Instance = OCTOSPI2;
  hospi_flash_ns.Init.FifoThreshold = 4;
  hospi_flash_ns.Init.DualQuad = HAL_OSPI_DUALQUAD_DISABLE;
  hospi_flash_ns.Init.MemoryType = HAL_OSPI_MEMTYPE_MACRONIX;
  hospi_flash_ns.Init.DeviceSize = 26;
  hospi_flash_ns.Init.ChipSelectHighTime = 2;
  hospi_flash_ns.Init.FreeRunningClock = HAL_OSPI_FREERUNCLK_DISABLE;
  hospi_flash_ns.Init.ClockMode = HAL_OSPI_CLOCK_MODE_0;
  hospi_flash_ns.Init.WrapSize = HAL_OSPI_WRAP_NOT_SUPPORTED;
  hospi_flash_ns.Init.ClockPrescaler = 4;
  hospi_flash_ns.Init.SampleShifting = HAL_OSPI_SAMPLE_SHIFTING_NONE;
  hospi_flash_ns.Init.DelayHoldQuarterCycle = HAL_OSPI_DHQC_ENABLE;
  hospi_flash_ns.Init.ChipSelectBoundary = 0;
  hospi_flash_ns.Init.DelayBlockBypass = HAL_OSPI_DELAY_BLOCK_USED;
  hospi_flash_ns.Init.MaxTran = 0;
  hospi_flash_ns.Init.Refresh = 0;
  if (HAL_OSPI_Init(&hospi_flash_ns) != HAL_OK)
  {
    return HAL_ERROR;
  }

  sOspiManagerCfg.ClkPort = 2;
  sOspiManagerCfg.DQSPort = 2;
  sOspiManagerCfg.NCSPort = 2;
  sOspiManagerCfg.IOLowPort = HAL_OSPIM_IOPORT_2_LOW;
  sOspiManagerCfg.IOHighPort = HAL_OSPIM_IOPORT_2_HIGH;
  if (HAL_OSPIM_Config(&hospi_flash_ns, &sOspiManagerCfg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return HAL_ERROR;
  }

  dlyb.Units = 0;
  dlyb.PhaseSel = 0;
  if (HAL_OSPI_DLYB_SetConfig(&hospi_flash_ns, &dlyb) != HAL_OK)
  {
    return HAL_ERROR;
  }

  u585_ospi_flash_ready = 1U;
  return HAL_OK;
}

uint8_t U585_OSPI_Flash_IsReady(void)
{
  return u585_ospi_flash_ready;
}

HAL_StatusTypeDef U585_OSPI_Flash_ReadJedecId(uint8_t id[3])
{
  OSPI_RegularCmdTypeDef sCommand = {0};

  if ((u585_ospi_flash_ready == 0U) || (id == NULL))
  {
    return HAL_ERROR;
  }

  sCommand.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
  sCommand.FlashId = HAL_OSPI_FLASH_ID_1;
  sCommand.Instruction = 0x9FU; /* READ ID */
  sCommand.InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;
  sCommand.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
  sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  sCommand.AddressMode = HAL_OSPI_ADDRESS_NONE;
  sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode = HAL_OSPI_DATA_1_LINE;
  sCommand.NbData = 3;
  sCommand.DataDtrMode = HAL_OSPI_DATA_DTR_DISABLE;
  sCommand.DummyCycles = 0;
  sCommand.DQSMode = HAL_OSPI_DQS_DISABLE;
  sCommand.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;

  if (HAL_OSPI_Command(&hospi_flash_ns, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return HAL_ERROR;
  }
  if (HAL_OSPI_Receive(&hospi_flash_ns, id, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return HAL_ERROR;
  }
  return HAL_OK;
}

HAL_StatusTypeDef U585_OSPI_Psram_Init(void)
{
  OSPIM_CfgTypeDef sOspiManagerCfg = {0};
  HAL_OSPI_DLYB_CfgTypeDef dlyb = {0};
  OSPI_RegularCmdTypeDef sCommand = {0};

  u585_ospi_psram_ready = 0U;

  /* Use generic memory type for SPI bring-up; AP-octal comes later. */
  hospi_psram_ns.Instance = OCTOSPI1;
  hospi_psram_ns.Init.FifoThreshold = 1;
  hospi_psram_ns.Init.DualQuad = HAL_OSPI_DUALQUAD_DISABLE;
  hospi_psram_ns.Init.MemoryType = HAL_OSPI_MEMTYPE_MICRON;
  hospi_psram_ns.Init.DeviceSize = 23;
  hospi_psram_ns.Init.ChipSelectHighTime = 1;
  hospi_psram_ns.Init.FreeRunningClock = HAL_OSPI_FREERUNCLK_DISABLE;
  hospi_psram_ns.Init.ClockMode = HAL_OSPI_CLOCK_MODE_0;
  hospi_psram_ns.Init.WrapSize = HAL_OSPI_WRAP_NOT_SUPPORTED;
  hospi_psram_ns.Init.ClockPrescaler = 16;
  hospi_psram_ns.Init.SampleShifting = HAL_OSPI_SAMPLE_SHIFTING_HALFCYCLE;
  hospi_psram_ns.Init.DelayHoldQuarterCycle = HAL_OSPI_DHQC_DISABLE;
  hospi_psram_ns.Init.ChipSelectBoundary = 0;
  hospi_psram_ns.Init.DelayBlockBypass = HAL_OSPI_DELAY_BLOCK_BYPASSED;
  hospi_psram_ns.Init.MaxTran = 0;
  hospi_psram_ns.Init.Refresh = 0;
  if (HAL_OSPI_Init(&hospi_psram_ns) != HAL_OK)
  {
    return HAL_ERROR;
  }

  sOspiManagerCfg.ClkPort = 1;
  sOspiManagerCfg.DQSPort = 1;
  sOspiManagerCfg.NCSPort = 1;
  sOspiManagerCfg.IOLowPort = HAL_OSPIM_IOPORT_1_LOW;
  sOspiManagerCfg.IOHighPort = HAL_OSPIM_IOPORT_NONE;
  if (HAL_OSPIM_Config(&hospi_psram_ns, &sOspiManagerCfg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return HAL_ERROR;
  }

  dlyb.Units = 0;
  dlyb.PhaseSel = 0;
  (void)HAL_OSPI_DLYB_SetConfig(&hospi_psram_ns, &dlyb);

  /* APS6408 software reset: 0x66 then 0x99. */
  sCommand.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
  sCommand.FlashId = HAL_OSPI_FLASH_ID_1;
  sCommand.InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;
  sCommand.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
  sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  sCommand.AddressMode = HAL_OSPI_ADDRESS_NONE;
  sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode = HAL_OSPI_DATA_NONE;
  sCommand.DummyCycles = 0;
  sCommand.DQSMode = HAL_OSPI_DQS_DISABLE;
  sCommand.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;
  sCommand.Instruction = 0x66U;
  if (HAL_OSPI_Command(&hospi_psram_ns, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return HAL_ERROR;
  }
  sCommand.Instruction = 0x99U;
  if (HAL_OSPI_Command(&hospi_psram_ns, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return HAL_ERROR;
  }
  HAL_Delay(1U);

  u585_ospi_psram_ready = 1U;
  return HAL_OK;
}

uint8_t U585_OSPI_Psram_IsReady(void)
{
  return u585_ospi_psram_ready;
}

HAL_StatusTypeDef U585_OSPI_Psram_MemTest(uint32_t *wrote, uint32_t *readback)
{
  OSPI_RegularCmdTypeDef sCommand = {0};
  uint8_t id[2] = {0};
  uint8_t pattern[4] = {0xA5U, 0x5AU, 0x12U, 0x34U};
  uint8_t value[4] = {0};
  uint32_t i;

  if ((u585_ospi_psram_ready == 0U) || (wrote == NULL) || (readback == NULL))
  {
    return HAL_ERROR;
  }

  /* First prove the chip answers READ ID 0x9F (MFID expect 0x0D). */
  sCommand.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
  sCommand.FlashId = HAL_OSPI_FLASH_ID_1;
  sCommand.Instruction = 0x9FU;
  sCommand.InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;
  sCommand.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
  sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  sCommand.AddressMode = HAL_OSPI_ADDRESS_NONE;
  sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode = HAL_OSPI_DATA_1_LINE;
  sCommand.NbData = 2;
  sCommand.DataDtrMode = HAL_OSPI_DATA_DTR_DISABLE;
  sCommand.DummyCycles = 0;
  sCommand.DQSMode = HAL_OSPI_DQS_DISABLE;
  sCommand.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;
  if (HAL_OSPI_Command(&hospi_psram_ns, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return HAL_ERROR;
  }
  if (HAL_OSPI_Receive(&hospi_psram_ns, id, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return HAL_ERROR;
  }
  *wrote = ((uint32_t)id[0] << 8) | (uint32_t)id[1];

  sCommand.Instruction = 0x02U;
  sCommand.Address = 0U;
  sCommand.AddressMode = HAL_OSPI_ADDRESS_1_LINE;
  sCommand.AddressSize = HAL_OSPI_ADDRESS_24_BITS;
  sCommand.AddressDtrMode = HAL_OSPI_ADDRESS_DTR_DISABLE;
  sCommand.NbData = 4;
  if (HAL_OSPI_Command(&hospi_psram_ns, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return HAL_ERROR;
  }
  if (HAL_OSPI_Transmit(&hospi_psram_ns, pattern, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return HAL_ERROR;
  }

  sCommand.Instruction = 0x03U;
  if (HAL_OSPI_Command(&hospi_psram_ns, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return HAL_ERROR;
  }
  if (HAL_OSPI_Receive(&hospi_psram_ns, value, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return HAL_ERROR;
  }

  *readback = ((uint32_t)value[0] << 24) | ((uint32_t)value[1] << 16) |
              ((uint32_t)value[2] << 8) | (uint32_t)value[3];

  /* Pass if JEDEC MFID looks like AP Memory (0x0D), even if RW still pending. */
  if (id[0] == 0x0DU)
  {
    for (i = 0U; i < 4U; i++)
    {
      if (value[i] != pattern[i])
      {
        /* ID ok; RW mismatch — still report partial via readback, fail memtest. */
        return HAL_ERROR;
      }
    }
    return HAL_OK;
  }
  return HAL_ERROR;
}
