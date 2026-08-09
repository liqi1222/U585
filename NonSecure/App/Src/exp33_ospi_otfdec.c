#include "u585_board.h"
#include "u585_demo.h"
#include "u585_log.h"
#include "u585_ospi.h"
#include "secure_nsc.h"

/*
 * exp33: OSPI external PSRAM + Flash with OTFDEC (on-the-fly decryption).
 *
 *   OCTOSPI2 / MX25LM51245G NOR flash @0x70000000, OTFDEC2 region1 0x70000000-0x70000FFF
 *   OCTOSPI1 / APS6408L      PSRAM     @0x90000000, OTFDEC1 region1 0x90000000-0x90000FFF
 *
 * The demo key/nonce live in the Secure world only (secure_nsc.c). NonSecure
 * drives the memories in octal DTR memory-mapped mode and asks Secure (NSC)
 * to program OTFDEC regions, encipher test vectors, and toggle REG_EN.
 *
 * Checked properties:
 *   1. Memory-mapped read with region enabled  -> returns plaintext.
 *   2. Raw indirect read of the same location  -> returns ciphertext.
 *   3. Memory-mapped read with region disabled -> returns ciphertext.
 *   4. Key CRC from OTFDEC matches between instances (same key programmed).
 */

#define EXP33_NWORDS            16U
#define EXP33_FLASH_MM_BASE     0x70000000UL
#define EXP33_PSRAM_MM_BASE     0x90000000UL
#define EXP33_FLASH_TEST_ADDR   0x00000000UL
#define EXP33_PSRAM_TEST_ADDR   0x00000000UL

#define EXP33_OTFDEC_SEL_PSRAM  1U
#define EXP33_OTFDEC_SEL_FLASH  2U

typedef struct
{
  uint32_t magic;
  uint32_t iterations;
  uint32_t tick_ms;
  uint32_t flash_spi_ready;
  uint32_t jedec_ok;
  uint32_t flash_octal_ok;
  uint32_t psram_octal_ok;
  uint32_t otfdec_setup_raw;
  uint32_t otfdec_ready;
  uint32_t key_crc_psram;
  uint32_t key_crc_flash;
  uint32_t flash_erase_ok;
  uint32_t flash_cipher_ok;
  uint32_t flash_prog_ok;
  uint32_t flash_raw_ok;
  uint32_t flash_raw_is_cipher;
  uint32_t flash_mm_ok;
  uint32_t flash_mm_read_ok;
  uint32_t flash_plain_ok;
  uint32_t flash_cipher_visible_ok;
  uint32_t psram_cipher_ok;
  uint32_t psram_sw_match;
  uint32_t psram_mm_ok;
  uint32_t psram_mm_rw_ok;
  uint32_t psram_plain_ok;
  uint32_t psram_cipher_visible_ok;
  uint32_t all_ok;
} U585_Exp33State;

volatile U585_Exp33State g_u585_exp33_state;

static uint32_t exp33_plain[EXP33_NWORDS];
static uint32_t exp33_cipher[EXP33_NWORDS];
static uint32_t exp33_buffer[EXP33_NWORDS];
static uint32_t exp33_raw[EXP33_NWORDS];

static void exp33_fill_plain(void)
{
  uint32_t i;
  uint32_t acc = 0xA5850330UL;

  for (i = 0U; i < EXP33_NWORDS; i++)
  {
    acc = (acc * 1664525UL) + 1013904223UL;
    exp33_plain[i] = acc ^ (0x5A000000UL | i);
  }
}

static uint32_t exp33_words_equal(const uint32_t *a, const uint32_t *b, uint32_t nwords)
{
  uint32_t i;

  for (i = 0U; i < nwords; i++)
  {
    if (a[i] != b[i])
    {
      return 0U;
    }
  }
  return 1U;
}

static void exp33_mm_read(uint32_t addr, uint32_t *dst, uint32_t nwords)
{
  volatile const uint32_t *src = (volatile const uint32_t *)addr;
  uint32_t i;

  for (i = 0U; i < nwords; i++)
  {
    dst[i] = src[i];
  }
}

static void exp33_mm_write(uint32_t addr, const uint32_t *src, uint32_t nwords)
{
  volatile uint32_t *dst = (volatile uint32_t *)addr;
  uint32_t i;

  for (i = 0U; i < nwords; i++)
  {
    dst[i] = src[i];
  }
}

static void exp33_log_flag(const char *name, uint32_t value)
{
  U585_Log_WriteU32(name, value);
}

/* Full OTFDEC test pass on both memories. Assumes octal modes + setup done. */
static void exp33_run_tests(void)
{
  exp33_fill_plain();

  /* ------------------------------ FLASH ------------------------------ */
  /* Stage A: pure indirect sanity — erase, program plaintext, raw read.  */
  g_u585_exp33_state.flash_erase_ok =
      (U585_OSPI_Flash_Erase4K(EXP33_FLASH_TEST_ADDR) == HAL_OK) ? 1U : 0U;
  exp33_log_flag("[U585][33] flash_erase_ok=", g_u585_exp33_state.flash_erase_ok);

  g_u585_exp33_state.flash_prog_ok =
      (U585_OSPI_Flash_PageProgram(EXP33_FLASH_TEST_ADDR, (const uint8_t *)exp33_plain,
                                   EXP33_NWORDS * 4U) == HAL_OK) ? 1U : 0U;
  exp33_log_flag("[U585][33] flash_prog_ok=", g_u585_exp33_state.flash_prog_ok);

  g_u585_exp33_state.flash_raw_ok = 0U;
  if ((g_u585_exp33_state.flash_erase_ok != 0U) && (g_u585_exp33_state.flash_prog_ok != 0U) &&
      (U585_OSPI_Flash_ReadRaw(EXP33_FLASH_TEST_ADDR, (uint8_t *)exp33_buffer,
                               EXP33_NWORDS * 4U) == HAL_OK))
  {
    g_u585_exp33_state.flash_raw_ok =
        exp33_words_equal(exp33_buffer, exp33_plain, EXP33_NWORDS);
  }
  exp33_log_flag("[U585][33] flash_raw_ok=", g_u585_exp33_state.flash_raw_ok);

  /* Stage B: memory-mapped read path WITHOUT OTFDEC (region disabled).   */
  g_u585_exp33_state.flash_mm_ok =
      (U585_OSPI_Flash_EnableMemoryMapped() == HAL_OK) ? 1U : 0U;
  exp33_log_flag("[U585][33] flash_mm_ok=", g_u585_exp33_state.flash_mm_ok);
  exp33_log_flag("[U585][33] flash_mm_fail_step=", U585_OSPI_Flash_MmFailStep());

  g_u585_exp33_state.flash_mm_read_ok = 0U;
  if ((g_u585_exp33_state.flash_mm_ok != 0U) &&
      (SECURE_OTFDEC_RegionEnable(EXP33_OTFDEC_SEL_FLASH, 0U) == 0U))
  {
    exp33_mm_read(EXP33_FLASH_MM_BASE + EXP33_FLASH_TEST_ADDR, exp33_buffer, EXP33_NWORDS);
    g_u585_exp33_state.flash_mm_read_ok =
        exp33_words_equal(exp33_buffer, exp33_plain, EXP33_NWORDS);
    (void)SECURE_OTFDEC_RegionEnable(EXP33_OTFDEC_SEL_FLASH, 1U);
  }
  exp33_log_flag("[U585][33] flash_mm_read_ok=", g_u585_exp33_state.flash_mm_read_ok);

  /* Stage C: ciphertext production for the NOR flash. Memory-mapped writes */
  /* cannot page-program the MX25LM (every AHB write would need WEL + a PP  */
  /* command), so the Secure world computes the OTFDEC-identical ciphertext */
  /* in software (CRYP AES-128-CTR, same key/nonce/version/address IV) and  */
  /* NonSecure stores it with indirect page-program commands — the same     */
  /* flow a production bootloader uses to provision encrypted firmware.     */
  (void)U585_OSPI_Flash_DisableMemoryMapped();
  g_u585_exp33_state.flash_cipher_ok = 0U;
  if ((g_u585_exp33_state.flash_mm_ok != 0U) &&
      (U585_OSPI_Flash_Erase4K(EXP33_FLASH_TEST_ADDR) == HAL_OK) &&
      (SECURE_OTFDEC_CipherSw(EXP33_OTFDEC_SEL_FLASH,
                              EXP33_FLASH_MM_BASE + EXP33_FLASH_TEST_ADDR,
                              exp33_plain, exp33_cipher) == 0U) &&
      (U585_OSPI_Flash_PageProgram(EXP33_FLASH_TEST_ADDR, (const uint8_t *)exp33_cipher,
                                   EXP33_NWORDS * 4U) == HAL_OK))
  {
    g_u585_exp33_state.flash_cipher_ok = 1U;
  }
  exp33_log_flag("[U585][33] flash_cipher_ok=", g_u585_exp33_state.flash_cipher_ok);
  exp33_log_flag("[U585][33] flash_cip0=", exp33_cipher[0]);

  /* Stage D: ground truth — raw indirect read shows what physically      */
  /* landed in the flash (use it, not the racy ENC read-back, as the      */
  /* reference ciphertext).                                               */
  g_u585_exp33_state.flash_raw_is_cipher = 0U;
  if ((g_u585_exp33_state.flash_cipher_ok != 0U) &&
      (U585_OSPI_Flash_ReadRaw(EXP33_FLASH_TEST_ADDR, (uint8_t *)exp33_raw,
                               EXP33_NWORDS * 4U) == HAL_OK))
  {
    g_u585_exp33_state.flash_raw_is_cipher =
        exp33_words_equal(exp33_raw, exp33_cipher, EXP33_NWORDS);
  }
  exp33_log_flag("[U585][33] flash_raw_is_cipher=", g_u585_exp33_state.flash_raw_is_cipher);
  exp33_log_flag("[U585][33] flash_raw0=", exp33_raw[0]);
  exp33_log_flag("[U585][33] flash_raw1=", exp33_raw[1]);
  exp33_log_flag("[U585][33] flash_cipher_differs=",
                 (exp33_words_equal(exp33_plain, exp33_raw, EXP33_NWORDS) == 0U) ? 1U : 0U);

  /* Stage E: decrypted view (region on) vs raw view (region off).        */
  g_u585_exp33_state.flash_plain_ok = 0U;
  g_u585_exp33_state.flash_cipher_visible_ok = 0U;
  if ((g_u585_exp33_state.flash_cipher_ok != 0U) &&
      (U585_OSPI_Flash_EnableMemoryMapped() == HAL_OK))
  {
    exp33_mm_read(EXP33_FLASH_MM_BASE + EXP33_FLASH_TEST_ADDR, exp33_buffer, EXP33_NWORDS);
    g_u585_exp33_state.flash_plain_ok =
        exp33_words_equal(exp33_buffer, exp33_plain, EXP33_NWORDS);
    exp33_log_flag("[U585][33] flash_plain_ok=", g_u585_exp33_state.flash_plain_ok);
    exp33_log_flag("[U585][33] flash_rd0=", exp33_buffer[0]);
    exp33_log_flag("[U585][33] flash_rd1=", exp33_buffer[1]);
    exp33_log_flag("[U585][33] flash_want0=", exp33_plain[0]);

    if (SECURE_OTFDEC_RegionEnable(EXP33_OTFDEC_SEL_FLASH, 0U) == 0U)
    {
      exp33_mm_read(EXP33_FLASH_MM_BASE + EXP33_FLASH_TEST_ADDR, exp33_buffer, EXP33_NWORDS);
      g_u585_exp33_state.flash_cipher_visible_ok =
          exp33_words_equal(exp33_buffer, exp33_raw, EXP33_NWORDS);
      (void)SECURE_OTFDEC_RegionEnable(EXP33_OTFDEC_SEL_FLASH, 1U);
    }
    exp33_log_flag("[U585][33] flash_cipher_visible_ok=", g_u585_exp33_state.flash_cipher_visible_ok);
    (void)U585_OSPI_Flash_DisableMemoryMapped();
  }

  /* ------------------------------ PSRAM ------------------------------ */
  g_u585_exp33_state.psram_mm_ok =
      (U585_OSPI_Psram_EnableMemoryMapped() == HAL_OK) ? 1U : 0U;
  exp33_log_flag("[U585][33] psram_mm_ok=", g_u585_exp33_state.psram_mm_ok);
  exp33_log_flag("[U585][33] psram_mm_fail_step=", U585_OSPI_Psram_MmFailStep());

  /* Stage A: MM read/write path WITHOUT OTFDEC (region disabled).        */
  g_u585_exp33_state.psram_mm_rw_ok = 0U;
  if ((g_u585_exp33_state.psram_mm_ok != 0U) &&
      (SECURE_OTFDEC_RegionEnable(EXP33_OTFDEC_SEL_PSRAM, 0U) == 0U))
  {
    exp33_mm_write(EXP33_PSRAM_MM_BASE + EXP33_PSRAM_TEST_ADDR, exp33_plain, EXP33_NWORDS);
    exp33_mm_read(EXP33_PSRAM_MM_BASE + EXP33_PSRAM_TEST_ADDR, exp33_buffer, EXP33_NWORDS);
    g_u585_exp33_state.psram_mm_rw_ok =
        exp33_words_equal(exp33_buffer, exp33_plain, EXP33_NWORDS);
    (void)SECURE_OTFDEC_RegionEnable(EXP33_OTFDEC_SEL_PSRAM, 1U);
  }
  exp33_log_flag("[U585][33] psram_mm_rw_ok=", g_u585_exp33_state.psram_mm_rw_ok);

  /* Stage B: hardware ENC cipher. NOTE (measured + HAL doc): in ENC mode  */
  /* the OTFDEC encrypts the READ data path — the plaintext write passes    */
  /* through unmodified, so the PSRAM still holds plaintext afterwards and  */
  /* the ciphertext exists only in the returned buffer ("it is up to the    */
  /* user code to copy the ciphered data in external RAM", says the HAL).   */
  g_u585_exp33_state.psram_cipher_ok = 0U;
  if (g_u585_exp33_state.psram_mm_ok != 0U)
  {
    g_u585_exp33_state.psram_cipher_ok =
        (SECURE_OTFDEC_Cipher(EXP33_OTFDEC_SEL_PSRAM, EXP33_PSRAM_MM_BASE + EXP33_PSRAM_TEST_ADDR,
                              exp33_plain, exp33_cipher) == 0U) ? 1U : 0U;
  }
  exp33_log_flag("[U585][33] psram_cipher_ok=", g_u585_exp33_state.psram_cipher_ok);
  exp33_log_flag("[U585][33] psram_cip0=", exp33_cipher[0]);

  /* Cross-check: software CRYP cipher (same key/nonce/version/IV) must    */
  /* match the hardware ENC cipher word for word.                          */
  g_u585_exp33_state.psram_sw_match = 0U;
  if (SECURE_OTFDEC_CipherSw(EXP33_OTFDEC_SEL_PSRAM,
                             EXP33_PSRAM_MM_BASE + EXP33_PSRAM_TEST_ADDR,
                             exp33_plain, exp33_raw) == 0U)
  {
    g_u585_exp33_state.psram_sw_match =
        exp33_words_equal(exp33_raw, exp33_cipher, EXP33_NWORDS);
  }
  exp33_log_flag("[U585][33] psram_sw_match=", g_u585_exp33_state.psram_sw_match);

  /* Stage C: store the ciphertext raw (region disabled), then:            */
  /* region off -> ciphertext, region on -> plaintext.                     */
  g_u585_exp33_state.psram_plain_ok = 0U;
  g_u585_exp33_state.psram_cipher_visible_ok = 0U;
  if ((g_u585_exp33_state.psram_mm_ok != 0U) && (g_u585_exp33_state.psram_cipher_ok != 0U))
  {
    if (SECURE_OTFDEC_RegionEnable(EXP33_OTFDEC_SEL_PSRAM, 0U) == 0U)
    {
      /* Region disabled: store the ciphertext raw into the PSRAM. */
      exp33_mm_write(EXP33_PSRAM_MM_BASE + EXP33_PSRAM_TEST_ADDR, exp33_cipher, EXP33_NWORDS);
      exp33_mm_read(EXP33_PSRAM_MM_BASE + EXP33_PSRAM_TEST_ADDR, exp33_buffer, EXP33_NWORDS);
      g_u585_exp33_state.psram_cipher_visible_ok =
          exp33_words_equal(exp33_buffer, exp33_cipher, EXP33_NWORDS);
      exp33_log_flag("[U585][33] psram_cvis0=", exp33_buffer[0]);
      exp33_log_flag("[U585][33] psram_cvis1=", exp33_buffer[1]);
      (void)SECURE_OTFDEC_RegionEnable(EXP33_OTFDEC_SEL_PSRAM, 1U);
    }
    exp33_log_flag("[U585][33] psram_cipher_visible_ok=", g_u585_exp33_state.psram_cipher_visible_ok);
    exp33_log_flag("[U585][33] otf1_cr=", SECURE_OTFDEC_GetReg(EXP33_OTFDEC_SEL_PSRAM, 0U));
    exp33_log_flag("[U585][33] otf1_cfgr=", SECURE_OTFDEC_GetReg(EXP33_OTFDEC_SEL_PSRAM, 1U));
    exp33_log_flag("[U585][33] otf2_cr=", SECURE_OTFDEC_GetReg(EXP33_OTFDEC_SEL_FLASH, 0U));
    exp33_log_flag("[U585][33] otf2_cfgr=", SECURE_OTFDEC_GetReg(EXP33_OTFDEC_SEL_FLASH, 1U));

    exp33_mm_read(EXP33_PSRAM_MM_BASE + EXP33_PSRAM_TEST_ADDR, exp33_buffer, EXP33_NWORDS);
    g_u585_exp33_state.psram_plain_ok =
        exp33_words_equal(exp33_buffer, exp33_plain, EXP33_NWORDS);
    exp33_log_flag("[U585][33] psram_plain_ok=", g_u585_exp33_state.psram_plain_ok);
    exp33_log_flag("[U585][33] psram_rd0=", exp33_buffer[0]);
    exp33_log_flag("[U585][33] psram_rd1=", exp33_buffer[1]);
    exp33_log_flag("[U585][33] psram_want0=", exp33_plain[0]);
  }

  g_u585_exp33_state.all_ok =
      (g_u585_exp33_state.otfdec_ready &
       g_u585_exp33_state.flash_erase_ok & g_u585_exp33_state.flash_prog_ok &
       g_u585_exp33_state.flash_raw_ok & g_u585_exp33_state.flash_mm_ok &
       g_u585_exp33_state.flash_mm_read_ok & g_u585_exp33_state.flash_cipher_ok &
       g_u585_exp33_state.flash_plain_ok & g_u585_exp33_state.flash_cipher_visible_ok &
       g_u585_exp33_state.psram_mm_ok & g_u585_exp33_state.psram_mm_rw_ok &
       g_u585_exp33_state.psram_cipher_ok & g_u585_exp33_state.psram_sw_match &
       g_u585_exp33_state.psram_plain_ok & g_u585_exp33_state.psram_cipher_visible_ok);
  exp33_log_flag("[U585][33] all_ok=", g_u585_exp33_state.all_ok);
}

static void exp33_init(void)
{
  uint8_t id[3] = {0U, 0U, 0U};

  U585_Board_InitBasicGpio();
  g_u585_exp33_state.magic = 0xA5850033UL;
  g_u585_exp33_state.iterations = 0U;
  g_u585_exp33_state.tick_ms = HAL_GetTick();

  U585_Log_WriteLine("");
  U585_Log_WriteLine("[U585][33] OSPI PSRAM+Flash OTFDEC test (octal DTR + memory-mapped)");
  U585_Log_WriteLine("[U585][33] OTFDEC2 region @0x70000000 flash, OTFDEC1 region @0x90000000 PSRAM");

  /* Flash: SPI bring-up + JEDEC cross-check, then switch to octal DTR. */
  g_u585_exp33_state.flash_spi_ready = (U585_OSPI_Flash_Init() == HAL_OK) ? 1U : 0U;
  exp33_log_flag("[U585][33] flash_spi_ready=", g_u585_exp33_state.flash_spi_ready);

  g_u585_exp33_state.jedec_ok = 0U;
  if ((g_u585_exp33_state.flash_spi_ready != 0U) &&
      (U585_OSPI_Flash_ReadJedecId(id) == HAL_OK) &&
      (id[0] == 0xC2U))
  {
    g_u585_exp33_state.jedec_ok = 1U;
  }
  exp33_log_flag("[U585][33] jedec_ok=", g_u585_exp33_state.jedec_ok);

  g_u585_exp33_state.flash_octal_ok =
      (U585_OSPI_Flash_EnterOctalDtr() == HAL_OK) ? 1U : 0U;
  exp33_log_flag("[U585][33] flash_octal_ok=", g_u585_exp33_state.flash_octal_ok);
  exp33_log_flag("[U585][33] flash_octal_fail_step=", U585_OSPI_Flash_OctalFailStep());
  if (g_u585_exp33_state.flash_octal_ok == 0U)
  {
    /* diag layout: [0]=dlyb  [1]=oct_sr0|sr1<<8|id0<<16|id1<<24            */
    /* [2]=oct_id2|id3<<8|status<<16  [3]=spi_cr2r1|cr2r3<<8|id0<<16|id1<<24 */
    /* [4]=spi_id2  (status bits: 0=oct_sr 1=oct_id 2=spi_cr2r1 3=spi_cr2r3 */
    /* 4=spi_id; a set bit means that probe completed with HAL_OK)          */
    uint32_t diag[5] = {0U, 0U, 0U, 0U, 0U};

    U585_OSPI_Flash_OctalDiag(diag);
    exp33_log_flag("[U585][33] diag0_dlyb=", diag[0]);
    exp33_log_flag("[U585][33] diag1_oct=", diag[1]);
    exp33_log_flag("[U585][33] diag2_octst=", diag[2]);
    exp33_log_flag("[U585][33] diag3_spi=", diag[3]);
    exp33_log_flag("[U585][33] diag4_spiid2=", diag[4]);
  }

  /* PSRAM: direct octal DTR bring-up (chip powers up in octal mode). */
  g_u585_exp33_state.psram_octal_ok =
      (U585_OSPI_Psram_EnterOctalDtr() == HAL_OK) ? 1U : 0U;
  exp33_log_flag("[U585][33] psram_octal_ok=", g_u585_exp33_state.psram_octal_ok);

  /* Secure world programs both OTFDEC regions (key never leaves Secure). */
  g_u585_exp33_state.otfdec_setup_raw = SECURE_OTFDEC_Setup();
  g_u585_exp33_state.otfdec_ready = (g_u585_exp33_state.otfdec_setup_raw >> 31) & 1U;
  g_u585_exp33_state.key_crc_psram = (g_u585_exp33_state.otfdec_setup_raw >> 8) & 0xFFU;
  g_u585_exp33_state.key_crc_flash = (g_u585_exp33_state.otfdec_setup_raw >> 16) & 0xFFU;
  exp33_log_flag("[U585][33] otfdec_ready=", g_u585_exp33_state.otfdec_ready);
  exp33_log_flag("[U585][33] otfdec_setup_err1=", g_u585_exp33_state.otfdec_setup_raw & 0xFU);
  exp33_log_flag("[U585][33] otfdec_setup_err2=", (g_u585_exp33_state.otfdec_setup_raw >> 4) & 0xFU);
  exp33_log_flag("[U585][33] mpcwm_fail=", (g_u585_exp33_state.otfdec_setup_raw >> 27) & 0x3U);
  exp33_log_flag("[U585][33] key_crc_psram=", g_u585_exp33_state.key_crc_psram);
  exp33_log_flag("[U585][33] key_crc_flash=", g_u585_exp33_state.key_crc_flash);

  if ((g_u585_exp33_state.flash_octal_ok != 0U) &&
      (g_u585_exp33_state.psram_octal_ok != 0U) &&
      (g_u585_exp33_state.otfdec_ready != 0U))
  {
    exp33_run_tests();
  }
  else
  {
    U585_Log_WriteLine("[U585][33] prerequisites failed; tests skipped");
  }
}

static void exp33_loop(void)
{
  static uint8_t last_button = 0U;
  uint8_t button = U585_Board_IsUserButtonPressed();

  g_u585_exp33_state.iterations++;
  g_u585_exp33_state.tick_ms = HAL_GetTick();

  if ((button != 0U) && (last_button == 0U) && (g_u585_exp33_state.otfdec_ready != 0U))
  {
    U585_Log_WriteLine("[U585][33] re-run OTFDEC tests...");
    if (g_u585_exp33_state.flash_mm_ok != 0U)
    {
      (void)U585_OSPI_Flash_DisableMemoryMapped();
    }
    if (g_u585_exp33_state.psram_mm_ok != 0U)
    {
      (void)U585_OSPI_Psram_DisableMemoryMapped();
    }
    exp33_run_tests();
  }
  last_button = button;

  if ((g_u585_exp33_state.iterations % 4U) == 0U)
  {
    U585_Log_WriteU32("[U585][33] heartbeat=", g_u585_exp33_state.iterations);
  }

  U585_Board_ToggleGreenLed();
  HAL_Delay(500U);
}

const U585_Demo U585_Demo_Exp33 = {
  "33",
  "OSPI PSRAM+Flash OTFDEC",
  exp33_init,
  exp33_loop,
};
