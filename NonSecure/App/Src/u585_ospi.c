#include "u585_ospi.h"

OSPI_HandleTypeDef hospi_flash_ns;
OSPI_HandleTypeDef hospi_psram_ns;

static uint8_t u585_ospi_flash_ready = 0U;
static uint8_t u585_ospi_psram_ready = 0U;

/* Defined in the exp33 section below; tuned right after each HAL_OSPI_Init. */
static HAL_StatusTypeDef u585_ospi_dlyb_enable(OSPI_HandleTypeDef *hospi);

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

  /* Static DLYB sampling point from the official OSPI_NOR_AutoPolling_DTR   */
  /* example (.ioc: Units=56, PhaseSel=2 @160MHz/prescaler 4); runtime DLYB  */
  /* tuning right after init refines it (only works pre-traffic).            */
  dlyb.Units = 56;
  dlyb.PhaseSel = 2;
  if (HAL_OSPI_DLYB_SetConfig(&hospi_flash_ns, &dlyb) != HAL_OK)
  {
    return HAL_ERROR;
  }
  (void)u585_ospi_dlyb_enable(&hospi_flash_ns);

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

/* -------------------------------------------------------------------------- */
/* exp33: octal DTR (8D-8D-8D) + memory-mapped + OTFDEC support               */
/* Sequences follow ST BSP (b_u585i_iot02a_ospi.c, mx25lm51245g.c,            */
/* OSPI_PSRAM_MemoryMapped example) for the B-U585I-IOT02A memories:          */
/*   - OCTOSPI2: MX25LM51245G 64 MB NOR flash @0x70000000                     */
/*   - OCTOSPI1: APS6408L     8 MB PSRAM     @0x90000000                      */
/* -------------------------------------------------------------------------- */

/* MX25LM51245G commands */
#define U585_MX25_SPI_WREN            0x06U
#define U585_MX25_SPI_WRITE_CFG2      0x72U
#define U585_MX25_SPI_READ_CFG2       0x71U
#define U585_MX25_CR2_REG1_ADDR       0x00000000UL
#define U585_MX25_CR2_DOPI            0x02U
#define U585_MX25_CR2_REG3_ADDR       0x00000300UL
#define U585_MX25_CR2_DC_6_CYCLES     0x07U
#define U585_MX25_WRITE_REG_MAX_MS    40U

#define U585_MX25_OCTA_READ_STATUS    0x05FAU
#define U585_MX25_OCTA_WREN           0x06F9U
#define U585_MX25_OCTA_READ_CFG2      0x718EU
#define U585_MX25_OCTA_READ_DTR       0xEE11U
#define U585_MX25_OCTA_PAGE_PROG      0x12EDU
#define U585_MX25_OCTA_ERASE_4K       0x21DEU

/* Dummy cycles in DOPI mode: the flash keeps its POWER-ON default DC       */
/* (the official OSPI_NOR_AutoPolling_DTR example never reprograms CR2      */
/* reg3), so every register/status and memory read uses 20 dummy cycles,    */
/* exactly like the proven example flow. Do NOT write DC=6 + use 4/6 — a    */
/* silently dropped reg3 write leaves the flash expecting 20 and kills the  */
/* first poll.                                                              */
#define U585_MX25_DUMMY_READ_DTR      20U
#define U585_MX25_DUMMY_REG_DTR       20U
#define U585_MX25_SR_WIP              0x01U
#define U585_MX25_SR_WEL              0x02U
#define U585_MX25_AUTOPOLL_INTERVAL   0x10U

/* APS6408L mode registers / commands */
#define U585_APS_MR0_ADDR             0x00UL
#define U585_APS_MR0_VALUE            0x24U
#define U585_APS_MR8_ADDR             0x08UL
#define U585_APS_MR8_VALUE            0x0BU
#define U585_APS_WRITE_REG_CMD        0xC0U
#define U585_APS_READ_REG_CMD         0x40U
#define U585_APS_REG_READ_LATENCY     6U   /* register read latency code      */
#define U585_APS_MM_WRITE_CMD         0x8080U
#define U585_APS_MM_READ_CMD          0x0000U
#define U585_APS_MM_DUMMY_WRITE       4U
#define U585_APS_MM_DUMMY_READ        8U

static uint8_t u585_ospi_flash_octal_ready = 0U;
static uint8_t u585_ospi_psram_octal_ready = 0U;
static uint32_t u585_flash_mm_fail_step = 0U;
static uint32_t u585_psram_mm_fail_step = 0U;

uint32_t U585_OSPI_Flash_MmFailStep(void)
{
  return u585_flash_mm_fail_step;
}

uint32_t U585_OSPI_Psram_MmFailStep(void)
{
  return u585_psram_mm_fail_step;
}

uint8_t U585_OSPI_Flash_IsOctalReady(void)
{
  return u585_ospi_flash_octal_ready;
}

uint8_t U585_OSPI_Psram_IsOctalReady(void)
{
  return u585_ospi_psram_octal_ready;
}

/* DLYB tuning used before entering memory-mapped DTR operation. */
static uint32_t u585_ospi_dlyb_diag[3] = {0U, 0U, 0U};

void U585_OSPI_DlybDiag(uint32_t out[3])
{
  if (out != NULL)
  {
    out[0] = u585_ospi_dlyb_diag[0];
    out[1] = u585_ospi_dlyb_diag[1];
    out[2] = u585_ospi_dlyb_diag[2];
  }
}

static HAL_StatusTypeDef u585_ospi_dlyb_enable(OSPI_HandleTypeDef *hospi)
{
  HAL_OSPI_DLYB_CfgTypeDef cfg = {0};
  HAL_OSPI_DLYB_CfgTypeDef want = {0};
  HAL_OSPI_DLYB_CfgTypeDef check = {0};

  u585_ospi_dlyb_diag[0] = 0U;
  u585_ospi_dlyb_diag[1] = 0U;
  u585_ospi_dlyb_diag[2] = 0U;

  if (HAL_OSPI_DLYB_GetClockPeriod(hospi, &cfg) != HAL_OK)
  {
    u585_ospi_dlyb_diag[0] = 1U;
    return HAL_ERROR;
  }
  u585_ospi_dlyb_diag[1] = (cfg.PhaseSel << 16) | cfg.Units;
  /* Empirical value from ST BSP/examples: divide the phase by 4 in DTR. */
  cfg.PhaseSel /= 4U;
  want = cfg;
  if (HAL_OSPI_DLYB_SetConfig(hospi, &cfg) != HAL_OK)
  {
    u585_ospi_dlyb_diag[0] = 2U;
    return HAL_ERROR;
  }
  if (HAL_OSPI_DLYB_GetConfig(hospi, &check) != HAL_OK)
  {
    u585_ospi_dlyb_diag[0] = 3U;
    return HAL_ERROR;
  }
  u585_ospi_dlyb_diag[2] = (check.PhaseSel << 16) | check.Units;
  if ((check.PhaseSel != want.PhaseSel) || (check.Units != want.Units))
  {
    u585_ospi_dlyb_diag[0] = 4U;
    return HAL_ERROR;
  }
  return HAL_OK;
}

/* --- Flash (OCTOSPI2, MX25LM51245G) helpers in octal DTR mode ------------- */

static void u585_flash_octa_cmd(OSPI_RegularCmdTypeDef *cmd, uint16_t instruction,
                                uint32_t addr, uint32_t nbdata, uint32_t dummy, uint32_t dqs)
{
  cmd->OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  cmd->FlashId            = HAL_OSPI_FLASH_ID_1;
  cmd->Instruction        = instruction;
  cmd->InstructionMode    = HAL_OSPI_INSTRUCTION_8_LINES;
  cmd->InstructionSize    = HAL_OSPI_INSTRUCTION_16_BITS;
  cmd->InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_ENABLE;
  cmd->AddressMode        = HAL_OSPI_ADDRESS_8_LINES;
  cmd->AddressSize        = HAL_OSPI_ADDRESS_32_BITS;
  cmd->AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_ENABLE;
  cmd->Address            = addr;
  cmd->AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  cmd->DataMode           = (nbdata != 0U) ? HAL_OSPI_DATA_8_LINES : HAL_OSPI_DATA_NONE;
  cmd->NbData             = nbdata;
  cmd->DataDtrMode        = HAL_OSPI_DATA_DTR_ENABLE;
  cmd->DummyCycles        = dummy;
  cmd->DQSMode            = dqs;
  cmd->SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;
}

static HAL_StatusTypeDef u585_flash_status_poll(uint32_t match, uint32_t mask)
{
  OSPI_RegularCmdTypeDef cmd = {0};
  OSPI_AutoPollingTypeDef cfg = {0};

  u585_flash_octa_cmd(&cmd, U585_MX25_OCTA_READ_STATUS, 0U, 2U,
                      U585_MX25_DUMMY_REG_DTR, HAL_OSPI_DQS_ENABLE);
  if (HAL_OSPI_Command(&hospi_flash_ns, &cmd, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return HAL_ERROR;
  }

  cfg.Match         = match;
  cfg.Mask          = mask;
  cfg.MatchMode     = HAL_OSPI_MATCH_MODE_AND;
  cfg.Interval      = U585_MX25_AUTOPOLL_INTERVAL;
  cfg.AutomaticStop = HAL_OSPI_AUTOMATIC_STOP_ENABLE;
  return HAL_OSPI_AutoPolling(&hospi_flash_ns, &cfg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE);
}

static HAL_StatusTypeDef u585_flash_write_enable_dtr(void)
{
  OSPI_RegularCmdTypeDef cmd = {0};

  cmd.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  cmd.FlashId            = HAL_OSPI_FLASH_ID_1;
  cmd.Instruction        = U585_MX25_OCTA_WREN;
  cmd.InstructionMode    = HAL_OSPI_INSTRUCTION_8_LINES;
  cmd.InstructionSize    = HAL_OSPI_INSTRUCTION_16_BITS;
  cmd.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_ENABLE;
  cmd.AddressMode        = HAL_OSPI_ADDRESS_NONE;
  cmd.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  cmd.DataMode           = HAL_OSPI_DATA_NONE;
  cmd.DummyCycles        = 0U;
  cmd.DQSMode            = HAL_OSPI_DQS_DISABLE;
  cmd.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;
  if (HAL_OSPI_Command(&hospi_flash_ns, &cmd, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return HAL_ERROR;
  }
  /* Wait for the write-enable latch. */
  return u585_flash_status_poll(U585_MX25_SR_WEL, U585_MX25_SR_WEL);
}

static HAL_StatusTypeDef u585_flash_spi_cmd1(uint8_t instruction, uint32_t addr,
                                             uint32_t addr_lines, uint8_t *data, uint32_t nbdata)
{
  OSPI_RegularCmdTypeDef cmd = {0};

  cmd.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  cmd.FlashId            = HAL_OSPI_FLASH_ID_1;
  cmd.Instruction        = instruction;
  cmd.InstructionMode    = HAL_OSPI_INSTRUCTION_1_LINE;
  cmd.InstructionSize    = HAL_OSPI_INSTRUCTION_8_BITS;
  cmd.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  cmd.AddressMode        = addr_lines;
  cmd.AddressSize        = HAL_OSPI_ADDRESS_32_BITS;
  cmd.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_DISABLE;
  cmd.Address            = addr;
  cmd.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  cmd.DataMode           = (nbdata != 0U) ? HAL_OSPI_DATA_1_LINE : HAL_OSPI_DATA_NONE;
  cmd.NbData             = nbdata;
  cmd.DataDtrMode        = HAL_OSPI_DATA_DTR_DISABLE;
  cmd.DummyCycles        = 0U;
  cmd.DQSMode            = HAL_OSPI_DQS_DISABLE;
  cmd.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;
  if (HAL_OSPI_Command(&hospi_flash_ns, &cmd, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return HAL_ERROR;
  }
  if ((nbdata != 0U) && (data != NULL))
  {
    return HAL_OSPI_Transmit(&hospi_flash_ns, data, HAL_OSPI_TIMEOUT_DEFAULT_VALUE);
  }
  return HAL_OK;
}

/**
  * @brief  Switch MX25LM51245G from SPI (1-1-1) to octal DTR (8D-8D-8D).
  *         Requires U585_OSPI_Flash_Init() first (JEDEC cross-check).
  *         Mirrors OSPI_OctalDtrModeCfg() of the official B-U585I-IOT02A
  *         OSPI_NOR_AutoPolling_DTR example: single WRCR2(0, DOPI), 40ms
  *         delay, no controller re-init, 20 dummy cycles (default DC).
  *         On failure, U585_OSPI_Flash_OctalFailStep() reports the stage:
  *         1=SPI WREN/WEL  2=WRCR2 DOPI  3=retry WREN/WEL  4=retry WRCR2
  *         6=octal WIP poll  7=octal RDCR2  8=DOPI mismatch
  *         (DLYB tuning errors are not fatal; see U585_OSPI_Flash_OctalDiag)
  */
static uint32_t u585_flash_octal_fail_step = 0U;

uint32_t U585_OSPI_Flash_OctalFailStep(void)
{
  return u585_flash_octal_fail_step;
}

/* Diagnostics captured when entering octal DTR fails. The capture probes   */
/* both protocols: octal RDSR/RDID (does the bus work?) and SPI RDCR2/RDID  */
/* (is the flash actually still in SPI mode, i.e. did the DOPI write        */
/* silently fail?). Packed into 5 words, see U585_OSPI_Flash_OctalDiag.     */
static uint8_t  u585_flash_diag_sr[2] = {0U, 0U};      /* octal RDSR bytes   */
static uint8_t  u585_flash_diag_id[4] = {0U, 0U, 0U, 0U}; /* octal RDID      */
static uint8_t  u585_flash_diag_cr2[2] = {0U, 0U};     /* SPI RDCR2 reg1/reg3*/
static uint8_t  u585_flash_diag_spiid[3] = {0U, 0U, 0U}; /* SPI RDID         */
static uint32_t u585_flash_diag_status = 0U;           /* per-probe HAL_OK   */
static uint32_t u585_flash_diag_dlyb = 0U; /* bit31=tune error, [15:8]=PhaseSel, [6:0]=Units */

void U585_OSPI_Flash_OctalDiag(uint32_t out[5])
{
  if (out == NULL)
  {
    return;
  }
  out[0] = u585_flash_diag_dlyb;
  out[1] = (uint32_t)u585_flash_diag_sr[0] | ((uint32_t)u585_flash_diag_sr[1] << 8) |
           ((uint32_t)u585_flash_diag_id[0] << 16) | ((uint32_t)u585_flash_diag_id[1] << 24);
  out[2] = (uint32_t)u585_flash_diag_id[2] | ((uint32_t)u585_flash_diag_id[3] << 8) |
           (u585_flash_diag_status << 16);
  out[3] = (uint32_t)u585_flash_diag_cr2[0] | ((uint32_t)u585_flash_diag_cr2[1] << 8) |
           ((uint32_t)u585_flash_diag_spiid[0] << 16) | ((uint32_t)u585_flash_diag_spiid[1] << 24);
  out[4] = (uint32_t)u585_flash_diag_spiid[2];
}

/* Clean-state SPI probe: JEDEC ID + CR2 reg1/reg3. Answers the question    */
/* "is the memory still in SPI mode?" — once DOPI is active these reads     */
/* return zeros/garbage instead of C2/85/3A. Sets status bits 2/3/4.        */
static void u585_flash_spi_probe(void)
{
  OSPI_RegularCmdTypeDef scmd = {0};

  (void)HAL_OSPI_Abort(&hospi_flash_ns);

  scmd.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  scmd.FlashId            = HAL_OSPI_FLASH_ID_1;
  scmd.InstructionMode    = HAL_OSPI_INSTRUCTION_1_LINE;
  scmd.InstructionSize    = HAL_OSPI_INSTRUCTION_8_BITS;
  scmd.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  scmd.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  scmd.DataMode           = HAL_OSPI_DATA_1_LINE;
  scmd.DataDtrMode        = HAL_OSPI_DATA_DTR_DISABLE;
  scmd.DummyCycles        = 0U;
  scmd.DQSMode            = HAL_OSPI_DQS_DISABLE;
  scmd.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

  /* SPI RDID 0x9F, 3 bytes (no address). */
  scmd.Instruction = 0x9FU;
  scmd.AddressMode = HAL_OSPI_ADDRESS_NONE;
  scmd.NbData      = 3U;
  if ((HAL_OSPI_Command(&hospi_flash_ns, &scmd, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) &&
      (HAL_OSPI_Receive(&hospi_flash_ns, u585_flash_diag_spiid, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK))
  {
    u585_flash_diag_status |= 0x10U;
  }
  (void)HAL_OSPI_Abort(&hospi_flash_ns);

  /* SPI RDCR2 reg1 @0x00000000 (DOPI flag lives here). */
  scmd.Instruction = U585_MX25_SPI_READ_CFG2;
  scmd.AddressMode = HAL_OSPI_ADDRESS_1_LINE;
  scmd.AddressSize = HAL_OSPI_ADDRESS_32_BITS;
  scmd.Address     = U585_MX25_CR2_REG1_ADDR;
  scmd.NbData      = 1U;
  if ((HAL_OSPI_Command(&hospi_flash_ns, &scmd, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) &&
      (HAL_OSPI_Receive(&hospi_flash_ns, &u585_flash_diag_cr2[0], HAL_OSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK))
  {
    u585_flash_diag_status |= 0x04U;
  }
  (void)HAL_OSPI_Abort(&hospi_flash_ns);

  /* SPI RDCR2 reg3 @0x00000300 (dummy-cycle setting). */
  scmd.Address = U585_MX25_CR2_REG3_ADDR;
  if ((HAL_OSPI_Command(&hospi_flash_ns, &scmd, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) &&
      (HAL_OSPI_Receive(&hospi_flash_ns, &u585_flash_diag_cr2[1], HAL_OSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK))
  {
    u585_flash_diag_status |= 0x08U;
  }
  (void)HAL_OSPI_Abort(&hospi_flash_ns);
}

/* Post-failure probe in octal mode (bits 0/1); SPI probes run beforehand   */
/* via u585_flash_spi_probe() on a DeInit+Init-clean controller.            */
static void u585_flash_octal_diag_capture(void)
{
  OSPI_RegularCmdTypeDef cmd = {0};

  /* Reset the peripheral to a known-clean state first: a timed-out DQS    */
  /* receive can wedge the OSPI and poison every later probe.              */
  (void)HAL_OSPI_DeInit(&hospi_flash_ns);
  (void)HAL_OSPI_Init(&hospi_flash_ns);

  u585_flash_spi_probe();

  /* Octal RDSR 0x05FA, 4 dummy, 2 data bytes. */
  u585_flash_octa_cmd(&cmd, U585_MX25_OCTA_READ_STATUS, 0U, 2U,
                      U585_MX25_DUMMY_REG_DTR, HAL_OSPI_DQS_ENABLE);
  if ((HAL_OSPI_Command(&hospi_flash_ns, &cmd, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) &&
      (HAL_OSPI_Receive(&hospi_flash_ns, u585_flash_diag_sr, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK))
  {
    u585_flash_diag_status |= 0x01U;
  }
  (void)HAL_OSPI_Abort(&hospi_flash_ns);

  /* Octal RDID 0x9F60: instruction DTR, no address, 4 dummy, 4 bytes. */
  u585_flash_octa_cmd(&cmd, 0x9F60U, 0U, 4U, U585_MX25_DUMMY_REG_DTR, HAL_OSPI_DQS_ENABLE);
  cmd.AddressMode = HAL_OSPI_ADDRESS_NONE;
  if ((HAL_OSPI_Command(&hospi_flash_ns, &cmd, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) &&
      (HAL_OSPI_Receive(&hospi_flash_ns, u585_flash_diag_id, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK))
  {
    u585_flash_diag_status |= 0x02U;
  }
  (void)HAL_OSPI_Abort(&hospi_flash_ns);
}

/* SPI-mode (1-1-1) status register poll, used to confirm the WEL latch. */
static HAL_StatusTypeDef u585_flash_spi_status_poll(uint32_t match, uint32_t mask)
{
  OSPI_RegularCmdTypeDef cmd = {0};
  OSPI_AutoPollingTypeDef cfg = {0};

  cmd.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  cmd.FlashId            = HAL_OSPI_FLASH_ID_1;
  cmd.Instruction        = 0x05U; /* READ STATUS REGISTER, SPI mode */
  cmd.InstructionMode    = HAL_OSPI_INSTRUCTION_1_LINE;
  cmd.InstructionSize    = HAL_OSPI_INSTRUCTION_8_BITS;
  cmd.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  cmd.AddressMode        = HAL_OSPI_ADDRESS_NONE;
  cmd.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  cmd.DataMode           = HAL_OSPI_DATA_1_LINE;
  cmd.NbData             = 1U;
  cmd.DataDtrMode        = HAL_OSPI_DATA_DTR_DISABLE;
  cmd.DummyCycles        = 0U;
  cmd.DQSMode            = HAL_OSPI_DQS_DISABLE;
  cmd.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;
  if (HAL_OSPI_Command(&hospi_flash_ns, &cmd, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return HAL_ERROR;
  }

  cfg.Match         = match;
  cfg.Mask          = mask;
  cfg.MatchMode     = HAL_OSPI_MATCH_MODE_AND;
  cfg.Interval      = U585_MX25_AUTOPOLL_INTERVAL;
  cfg.AutomaticStop = HAL_OSPI_AUTOMATIC_STOP_ENABLE;
  return HAL_OSPI_AutoPolling(&hospi_flash_ns, &cfg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE);
}

static HAL_StatusTypeDef u585_flash_spi_wren(void)
{
  /* Wait out any in-flight register write first: a WREN sent while WIP=1  */
  /* is silently ignored by the memory, which previously let the following */
  /* WRCR2 (DOPI switch) get dropped even though the WEL poll "passed".    */
  if (u585_flash_spi_status_poll(0U, U585_MX25_SR_WIP) != HAL_OK)
  {
    return HAL_ERROR;
  }
  if (u585_flash_spi_cmd1(U585_MX25_SPI_WREN, 0U, HAL_OSPI_ADDRESS_NONE, NULL, 0U) != HAL_OK)
  {
    return HAL_ERROR;
  }
  return u585_flash_spi_status_poll(U585_MX25_SR_WEL, U585_MX25_SR_WEL);
}

HAL_StatusTypeDef U585_OSPI_Flash_EnterOctalDtr(void)
{
  uint8_t value;
  uint8_t reg[2] = {0U, 0U};

  u585_ospi_flash_octal_ready = 0U;
  u585_flash_octal_fail_step = 0U;
  if (u585_ospi_flash_ready == 0U)
  {
    return HAL_ERROR;
  }

  /* Mirror the proven OSPI_NOR_AutoPolling_DTR example flow byte-for-byte: */
  /* single WRCR2(0, DOPI) + 40ms delay, no CR2 reg3 write, no controller   */
  /* re-init, no DLYB re-tune (static 56/2 set at Flash_Init stays active), */
  /* and 20 dummy cycles (power-on default DC) for every octal read.        */

  /* 1. Enable DTR octal protocol (CR2 reg1 = DOPI). */
  if (u585_flash_spi_wren() != HAL_OK)
  {
    u585_flash_octal_fail_step = 1U;
    return HAL_ERROR;
  }
  value = U585_MX25_CR2_DOPI;
  if (u585_flash_spi_cmd1(U585_MX25_SPI_WRITE_CFG2, U585_MX25_CR2_REG1_ADDR,
                          HAL_OSPI_ADDRESS_1_LINE, &value, 1U) != HAL_OK)
  {
    u585_flash_octal_fail_step = 2U;
    return HAL_ERROR;
  }
  HAL_Delay(U585_MX25_WRITE_REG_MAX_MS);

  /* 1b. Clean-state verdict BEFORE any octal traffic: if the memory still */
  /* answers SPI RDID with C2/85/3A, the DOPI write was dropped (e.g. it   */
  /* raced a WIP window) — retry the write once and probe again.           */
  u585_flash_diag_status = 0U;
  u585_flash_spi_probe();
  if (u585_flash_diag_spiid[0] == 0xC2U)
  {
    if (u585_flash_spi_wren() != HAL_OK)
    {
      u585_flash_octal_fail_step = 3U;
      return HAL_ERROR;
    }
    value = U585_MX25_CR2_DOPI;
    if (u585_flash_spi_cmd1(U585_MX25_SPI_WRITE_CFG2, U585_MX25_CR2_REG1_ADDR,
                            HAL_OSPI_ADDRESS_1_LINE, &value, 1U) != HAL_OK)
    {
      u585_flash_octal_fail_step = 4U;
      return HAL_ERROR;
    }
    HAL_Delay(U585_MX25_WRITE_REG_MAX_MS);
    u585_flash_diag_status |= 0x8000U; /* mark: retry happened */
    u585_flash_spi_probe();
  }

  /* 2. Wait memory ready, then confirm CR2 reg1 reads back DOPI in octal. */
  if (u585_flash_status_poll(0U, U585_MX25_SR_WIP) != HAL_OK)
  {
    u585_flash_octal_fail_step = 6U;
    u585_flash_octal_diag_capture();
    return HAL_ERROR;
  }
  {
    OSPI_RegularCmdTypeDef cmd = {0};
    u585_flash_octa_cmd(&cmd, U585_MX25_OCTA_READ_CFG2, U585_MX25_CR2_REG1_ADDR, 2U,
                        U585_MX25_DUMMY_REG_DTR, HAL_OSPI_DQS_ENABLE);
    if (HAL_OSPI_Command(&hospi_flash_ns, &cmd, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      u585_flash_octal_fail_step = 7U;
      return HAL_ERROR;
    }
    if (HAL_OSPI_Receive(&hospi_flash_ns, reg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      u585_flash_octal_fail_step = 7U;
      return HAL_ERROR;
    }
  }
  if (reg[0] != U585_MX25_CR2_DOPI)
  {
    u585_flash_octal_fail_step = 8U;
    return HAL_ERROR;
  }

  u585_ospi_flash_octal_ready = 1U;
  return HAL_OK;
}

HAL_StatusTypeDef U585_OSPI_Flash_WriteEnableOctal(void)
{
  if (u585_ospi_flash_octal_ready == 0U)
  {
    return HAL_ERROR;
  }
  return u585_flash_write_enable_dtr();
}

HAL_StatusTypeDef U585_OSPI_Flash_Erase4K(uint32_t addr)
{
  OSPI_RegularCmdTypeDef cmd = {0};

  if ((u585_ospi_flash_octal_ready == 0U) || ((addr & 0xFFFU) != 0U))
  {
    return HAL_ERROR;
  }
  if (u585_flash_write_enable_dtr() != HAL_OK)
  {
    return HAL_ERROR;
  }
  u585_flash_octa_cmd(&cmd, U585_MX25_OCTA_ERASE_4K, addr, 0U, 0U, HAL_OSPI_DQS_DISABLE);
  if (HAL_OSPI_Command(&hospi_flash_ns, &cmd, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return HAL_ERROR;
  }
  return u585_flash_status_poll(0U, U585_MX25_SR_WIP);
}

HAL_StatusTypeDef U585_OSPI_Flash_PageProgram(uint32_t addr, const uint8_t *data, uint32_t size)
{
  OSPI_RegularCmdTypeDef cmd = {0};

  if ((u585_ospi_flash_octal_ready == 0U) || (data == NULL) ||
      (size == 0U) || (size > 256U) || ((size & 1U) != 0U))
  {
    return HAL_ERROR;
  }
  if (u585_flash_write_enable_dtr() != HAL_OK)
  {
    return HAL_ERROR;
  }
  u585_flash_octa_cmd(&cmd, U585_MX25_OCTA_PAGE_PROG, addr, size, 0U, HAL_OSPI_DQS_DISABLE);
  if (HAL_OSPI_Command(&hospi_flash_ns, &cmd, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return HAL_ERROR;
  }
  if (HAL_OSPI_Transmit(&hospi_flash_ns, (uint8_t *)data, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return HAL_ERROR;
  }
  return u585_flash_status_poll(0U, U585_MX25_SR_WIP);
}

/**
  * @brief  Raw flash content read (indirect mode, bypasses OTFDEC view):
  *         use it to verify what is physically stored (ciphertext).
  */
HAL_StatusTypeDef U585_OSPI_Flash_ReadRaw(uint32_t addr, uint8_t *data, uint32_t size)
{
  OSPI_RegularCmdTypeDef cmd = {0};

  if ((u585_ospi_flash_octal_ready == 0U) || (data == NULL) ||
      (size == 0U) || ((size & 1U) != 0U))
  {
    return HAL_ERROR;
  }
  u585_flash_octa_cmd(&cmd, U585_MX25_OCTA_READ_DTR, addr, size,
                      U585_MX25_DUMMY_READ_DTR, HAL_OSPI_DQS_ENABLE);
  if (HAL_OSPI_Command(&hospi_flash_ns, &cmd, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return HAL_ERROR;
  }
  return HAL_OSPI_Receive(&hospi_flash_ns, data, HAL_OSPI_TIMEOUT_DEFAULT_VALUE);
}

HAL_StatusTypeDef U585_OSPI_Flash_EnableMemoryMapped(void)
{
  OSPI_RegularCmdTypeDef cmd = {0};
  OSPI_MemoryMappedTypeDef mmcfg = {0};

  u585_flash_mm_fail_step = 0U;
  if (u585_ospi_flash_octal_ready == 0U)
  {
    u585_flash_mm_fail_step = 1U;
    return HAL_ERROR;
  }
  /* DLYB was already tuned at Flash_Init time (right after HAL_OSPI_Init,  */
  /* the only point where GetClockPeriod reliably succeeds). Do NOT re-tune */
  /* here: once indirect DTR traffic happened, the period sweep times out.  */

  cmd.OperationType      = HAL_OSPI_OPTYPE_READ_CFG;
  cmd.FlashId            = HAL_OSPI_FLASH_ID_1;
  cmd.Instruction        = U585_MX25_OCTA_READ_DTR;
  cmd.InstructionMode    = HAL_OSPI_INSTRUCTION_8_LINES;
  cmd.InstructionSize    = HAL_OSPI_INSTRUCTION_16_BITS;
  cmd.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_ENABLE;
  cmd.AddressMode        = HAL_OSPI_ADDRESS_8_LINES;
  cmd.AddressSize        = HAL_OSPI_ADDRESS_32_BITS;
  cmd.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_ENABLE;
  cmd.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  cmd.DataMode           = HAL_OSPI_DATA_8_LINES;
  cmd.DataDtrMode        = HAL_OSPI_DATA_DTR_ENABLE;
  cmd.DummyCycles        = U585_MX25_DUMMY_READ_DTR;
  cmd.DQSMode            = HAL_OSPI_DQS_ENABLE;
  cmd.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;
  if (HAL_OSPI_Command(&hospi_flash_ns, &cmd, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    u585_flash_mm_fail_step = 3U;
    return HAL_ERROR;
  }

  cmd.OperationType = HAL_OSPI_OPTYPE_WRITE_CFG;
  cmd.Instruction   = U585_MX25_OCTA_PAGE_PROG;
  cmd.DummyCycles   = 0U;
  cmd.DQSMode       = HAL_OSPI_DQS_DISABLE;
  if (HAL_OSPI_Command(&hospi_flash_ns, &cmd, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    u585_flash_mm_fail_step = 4U;
    return HAL_ERROR;
  }

  mmcfg.TimeOutActivation = HAL_OSPI_TIMEOUT_COUNTER_DISABLE;
  if (HAL_OSPI_MemoryMapped(&hospi_flash_ns, &mmcfg) != HAL_OK)
  {
    u585_flash_mm_fail_step = 5U;
    return HAL_ERROR;
  }
  return HAL_OK;
}

HAL_StatusTypeDef U585_OSPI_Flash_DisableMemoryMapped(void)
{
  return HAL_OSPI_Abort(&hospi_flash_ns);
}

/* --- PSRAM (OCTOSPI1, APS6408L) helpers in octal DTR mode ----------------- */

static void u585_psram_reg_cmd(OSPI_RegularCmdTypeDef *cmd, uint8_t instruction,
                               uint32_t addr, uint32_t dummy, uint32_t dqs)
{
  cmd->OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  cmd->FlashId            = HAL_OSPI_FLASH_ID_1;
  cmd->Instruction        = instruction;
  cmd->InstructionMode    = HAL_OSPI_INSTRUCTION_8_LINES;
  cmd->InstructionSize    = HAL_OSPI_INSTRUCTION_8_BITS;
  cmd->InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  cmd->AddressMode        = HAL_OSPI_ADDRESS_8_LINES;
  cmd->AddressSize        = HAL_OSPI_ADDRESS_32_BITS;
  cmd->AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_ENABLE;
  cmd->Address            = addr;
  cmd->AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  cmd->DataMode           = HAL_OSPI_DATA_8_LINES;
  cmd->NbData             = 2U;
  cmd->DataDtrMode        = HAL_OSPI_DATA_DTR_ENABLE;
  cmd->DummyCycles        = dummy;
  cmd->DQSMode            = dqs;
  cmd->SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;
}

static HAL_StatusTypeDef u585_psram_write_reg(uint32_t addr, uint8_t value)
{
  OSPI_RegularCmdTypeDef cmd = {0};

  u585_psram_reg_cmd(&cmd, U585_APS_WRITE_REG_CMD, addr, 0U, HAL_OSPI_DQS_DISABLE);
  if (HAL_OSPI_Command(&hospi_psram_ns, &cmd, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return HAL_ERROR;
  }
  return HAL_OSPI_Transmit(&hospi_psram_ns, &value, HAL_OSPI_TIMEOUT_DEFAULT_VALUE);
}

static HAL_StatusTypeDef u585_psram_read_reg(uint32_t addr, uint8_t *value)
{
  OSPI_RegularCmdTypeDef cmd = {0};

  u585_psram_reg_cmd(&cmd, U585_APS_READ_REG_CMD, addr,
                     (U585_APS_REG_READ_LATENCY - 1U), HAL_OSPI_DQS_ENABLE);
  if (HAL_OSPI_Command(&hospi_psram_ns, &cmd, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return HAL_ERROR;
  }
  return HAL_OSPI_Receive(&hospi_psram_ns, value, HAL_OSPI_TIMEOUT_DEFAULT_VALUE);
}

/**
  * @brief  Bring the APS6408L PSRAM up in octal DTR mode and program its
  *         mode registers (MR0 latency/drive, MR8 burst). The chip powers
  *         up in octal mode on this board, so no SPI phase is needed.
  */
HAL_StatusTypeDef U585_OSPI_Psram_EnterOctalDtr(void)
{
  OSPIM_CfgTypeDef ospim_cfg = {0};
  uint8_t reg[2] = {0U, 0U};

  u585_ospi_psram_octal_ready = 0U;

  /* BSP-grade controller configuration for the AP octal RAM. */
  hospi_psram_ns.Instance                    = OCTOSPI1;
  hospi_psram_ns.Init.FifoThreshold          = 1;
  hospi_psram_ns.Init.DualQuad               = HAL_OSPI_DUALQUAD_DISABLE;
  hospi_psram_ns.Init.MemoryType             = HAL_OSPI_MEMTYPE_APMEMORY;
  hospi_psram_ns.Init.DeviceSize             = 23;
  hospi_psram_ns.Init.ChipSelectHighTime     = 1;
  hospi_psram_ns.Init.FreeRunningClock       = HAL_OSPI_FREERUNCLK_DISABLE;
  hospi_psram_ns.Init.ClockMode              = HAL_OSPI_CLOCK_MODE_0;
  hospi_psram_ns.Init.WrapSize               = HAL_OSPI_WRAP_NOT_SUPPORTED;
  hospi_psram_ns.Init.ClockPrescaler         = 2;
  hospi_psram_ns.Init.SampleShifting         = HAL_OSPI_SAMPLE_SHIFTING_NONE;
  hospi_psram_ns.Init.DelayHoldQuarterCycle  = HAL_OSPI_DHQC_ENABLE;
  hospi_psram_ns.Init.ChipSelectBoundary     = 10;
  hospi_psram_ns.Init.DelayBlockBypass       = HAL_OSPI_DELAY_BLOCK_USED;
  hospi_psram_ns.Init.MaxTran                = 0;
  hospi_psram_ns.Init.Refresh                = 100;
  if (HAL_OSPI_Init(&hospi_psram_ns) != HAL_OK)
  {
    return HAL_ERROR;
  }

  /* Full 8-line bus: port1 low AND high nibble. */
  ospim_cfg.ClkPort    = 1;
  ospim_cfg.DQSPort    = 1;
  ospim_cfg.NCSPort    = 1;
  ospim_cfg.IOLowPort  = HAL_OSPIM_IOPORT_1_LOW;
  ospim_cfg.IOHighPort = HAL_OSPIM_IOPORT_1_HIGH;
  if (HAL_OSPIM_Config(&hospi_psram_ns, &ospim_cfg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return HAL_ERROR;
  }

  /* Tune the sampling delay right after HAL_OSPI_Init (only reliable      */
  /* point — post-traffic GetClockPeriod times out); keep going on failure */
  /* since indirect register access already works with the default delay.  */
  (void)u585_ospi_dlyb_enable(&hospi_psram_ns);

  /* MR0: read latency code + drive strength; MR8: burst type/length. */
  if (u585_psram_write_reg(U585_APS_MR0_ADDR, U585_APS_MR0_VALUE) != HAL_OK)
  {
    return HAL_ERROR;
  }
  if (u585_psram_write_reg(U585_APS_MR8_ADDR, U585_APS_MR8_VALUE) != HAL_OK)
  {
    return HAL_ERROR;
  }
  if (u585_psram_read_reg(U585_APS_MR0_ADDR, reg) != HAL_OK)
  {
    return HAL_ERROR;
  }
  if (reg[0] != U585_APS_MR0_VALUE)
  {
    return HAL_ERROR;
  }
  if (u585_psram_read_reg(U585_APS_MR8_ADDR, reg) != HAL_OK)
  {
    return HAL_ERROR;
  }
  if (reg[0] != U585_APS_MR8_VALUE)
  {
    return HAL_ERROR;
  }

  u585_ospi_psram_octal_ready = 1U;
  u585_ospi_psram_ready = 1U;
  return HAL_OK;
}

HAL_StatusTypeDef U585_OSPI_Psram_EnableMemoryMapped(void)
{
  OSPI_RegularCmdTypeDef cmd = {0};
  OSPI_MemoryMappedTypeDef mmcfg = {0};

  u585_psram_mm_fail_step = 0U;
  if (u585_ospi_psram_octal_ready == 0U)
  {
    u585_psram_mm_fail_step = 1U;
    return HAL_ERROR;
  }
  /* DLYB already tuned at EnterOctalDtr time (right after HAL_OSPI_Init). */

  cmd.OperationType      = HAL_OSPI_OPTYPE_WRITE_CFG;
  cmd.FlashId            = HAL_OSPI_FLASH_ID_1;
  cmd.Instruction        = U585_APS_MM_WRITE_CMD;
  cmd.InstructionMode    = HAL_OSPI_INSTRUCTION_8_LINES;
  cmd.InstructionSize    = HAL_OSPI_INSTRUCTION_16_BITS;
  cmd.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_ENABLE;
  cmd.AddressMode        = HAL_OSPI_ADDRESS_8_LINES;
  cmd.AddressSize        = HAL_OSPI_ADDRESS_32_BITS;
  cmd.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_ENABLE;
  cmd.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  cmd.DataMode           = HAL_OSPI_DATA_8_LINES;
  cmd.DataDtrMode        = HAL_OSPI_DATA_DTR_ENABLE;
  cmd.DummyCycles        = U585_APS_MM_DUMMY_WRITE;
  cmd.DQSMode            = HAL_OSPI_DQS_ENABLE;
  cmd.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;
  if (HAL_OSPI_Command(&hospi_psram_ns, &cmd, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    u585_psram_mm_fail_step = 3U;
    return HAL_ERROR;
  }

  cmd.OperationType = HAL_OSPI_OPTYPE_READ_CFG;
  cmd.Instruction   = U585_APS_MM_READ_CMD;
  cmd.DummyCycles   = U585_APS_MM_DUMMY_READ;
  if (HAL_OSPI_Command(&hospi_psram_ns, &cmd, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    u585_psram_mm_fail_step = 4U;
    return HAL_ERROR;
  }

  mmcfg.TimeOutActivation = HAL_OSPI_TIMEOUT_COUNTER_DISABLE;
  if (HAL_OSPI_MemoryMapped(&hospi_psram_ns, &mmcfg) != HAL_OK)
  {
    u585_psram_mm_fail_step = 5U;
    return HAL_ERROR;
  }
  return HAL_OK;
}

HAL_StatusTypeDef U585_OSPI_Psram_DisableMemoryMapped(void)
{
  return HAL_OSPI_Abort(&hospi_psram_ns);
}
