# U585 Experiment Code Layout

This project keeps article-facing test code under the NonSecure application layer unless an experiment is explicitly about TrustZone.

## Default Boundary

- `Secure/` keeps TrustZone boot handoff, system clock/power setup, GTZC initial setup, and Secure boot banner on USART1.
- After the Secure banner, Secure **hands USART1 / TIM2 / RTC / I2C2 (/ SPI2 when demo needs it) to NonSecure** (GTZC NSEC), releases related GPIO security bits, clears EXTI13 secure bit, and routes demo IRQs to NS via `NVIC_SetTargetState`.
- `NonSecure/App/` owns article demos, board helpers, NS USART1 (`u585_usart1.c`), and VCP logging (`u585_log.c` uses NS UART when ready, NSC fallback otherwise).
- Articles 31 and 32 remain Secure/TF-M topics.
- Dual-image layout: `TZEN=1`, `SECBOOTADD0=0x0C000000`, Bank2 NS at `0x08100000`. Verified OB: `SECWM2_PSTRT=0x7F SECWM2_PEND=0x0`.
- LEDs PH6/PH7 are NonSecure (`GPIOH->SECCFGR` clears SEC6/SEC7).

## Project Baseline (Article 03)

Article 03 establishes the **CubeMX + VS Code + CMake** baseline used by every later experiment. The goal is a minimal, reproducible loop: configure → generate → build → download → debug → run.

### CubeMX Minimal Configuration

Open `U585.ioc` as the single source of truth. The baseline project targets:

- **MCU**: STM32U585AIIxQ (UFBGA169)
- **TrustZone**: enabled (`Mcu.ContextProject=TrustZoneEnabled`)
- **Toolchain**: CMake (`ProjectManager.TargetToolchain=CMake`)
- **Firmware package**: STM32Cube FW_U5 V1.8.0

For the first LED/button demo, only three user-visible items are required:

| Item | Setting | Notes |
| --- | --- | --- |
| Clock | `SYSCLK = 160 MHz` from PLL (MSI 4 MHz, M=1/N=80/R=2) | Conservative and stable; article 05 explores higher/lower choices. |
| Red LED | `PH6` = `GPIO_Output`, label `LED_RED`, pull-up, high speed | NonSecure pin (`CortexM33NS`). |
| Green LED | `PH7` = `GPIO_Output`, label `LED_GREEN`, pull-up, high speed | NonSecure pin (`CortexM33NS`). |
| User button | `PC13` = `GPIO_Input`, label `USER_Button`, no pull | NonSecure pin; unpressed level reads as `0` on this board. |

Keep the `.ioc` under Git. Regenerate code after any `.ioc` change; hand-written application code lives in `USER CODE BEGIN/END` regions or in `NonSecure/App/` so regeneration does not overwrite it.

### VS Code / CMake / Debug Setup

The workspace uses the **separated toolchain** described in article 03:

1. **Build**: CMake + Ninja + `arm-none-eabi-gcc` via `gcc-arm-none-eabi.cmake`.
2. **Download**: STM32CubeProgrammer (ST-LINK) or OpenOCD.
3. **Debug**: VS Code `cortex-debug` extension + ST-LINK GDB server.

Open `U585.code-workspace` (not a sub-folder) so the root preset builds `Secure` before `NonSecure`:

```powershell
cmake --preset Debug
cmake --build --preset Debug
```

For the baseline demo, leave `U585_ACTIVE_DEMO=3` (default in root `CMakeLists.txt`). F5 launches **U585 NonSecure (ST-LINK)**, programs both TrustZone images, and stops at `NonSecure/Core/Src/main.c:main`. The on-board ST-LINK/V3E handles download and debug at `1000 kHz` SWD. See [docs/VS_CODE.md](docs/VS_CODE.md) for extension list, path overrides, and troubleshooting.

### LED/Button Demo Behaviour

`NonSecure/App/Src/exp03_led_button.c`:

- Initial state: red LED on, green LED off.
- Each loop flips the two LEDs.
- Default blink period: 500 ms.
- Pressing the user button toggles the period between 500 ms and 100 ms (edge-detected).
- VCP prints `led_state=`, `heartbeat=`, and `button_edge_delay_ms=`.

### Article 03 Measured Record

**2026-07-12 (original article note)**

- **Board**: B-U585I-IOT02A, ST-LINK VCP **COM3 @ 115200 8N1**.
- **Build**: `U585_ACTIVE_DEMO=3`, default Debug preset.
- **Download**: STM32CubeProgrammer + VS Code `cortex-debug` (ST-LINK path; OpenOCD not used).
- **Observation**: red/green LEDs alternate; user button edge changes blink rate; VCP emits `[U585][03] led_state=` and `heartbeat=` lines.
- **Raw log**: `docs/superpowers/measured/demo03-com3.txt`.

**2026-08-07 (re-verified from this workspace)**

- **Toolchain**: CMake 3.28.1 + Ninja 1.11.1 + arm-none-eabi-gcc 14.3.1 (STM32CubeCLT 1.21.0).
- **Build**: clean configure/build of `Debug` preset succeeded (`U585_S.elf` 45.95 KB @ `0x0C000000`, `U585_NS.elf` 21.02 KB @ `0x08100000`).
- **Probe**: ST-LINK/V3E SN `001E003B4D46501220383832`, firmware V3J17M10, SWD 1000 kHz.
- **Download**: both images flashed and verified with `STM32_Programmer_CLI.exe`.
- **Run capture (COM3, 6 s)**:

```text
[S] Secure world ready
[NS] [U585][03] LED/button demo start
[NS] [U585][03] USART1 VCP log via NonSecure UART (post Phase-B handoff)
[NS] [U585][03] initial_button=0
[NS] [U585][03] led_state=2
[NS] [U585][03] led_state=1
[NS] [U585][03] led_state=2
...
[NS] [U585][03] heartbeat=10
```

- **Status**: ✅ LED/VCP loop running; download path confirmed from this checkout.

### Common Baseline Pitfalls

- **Editing generated files directly**: always change `.ioc` and regenerate, or keep code inside `USER CODE` fences.
- **Forgetting `.ioc` in Git**: without it the configuration cannot be reproduced on another machine.
- **Opening the sub-folder instead of the workspace**: the standalone `NonSecure` or `Secure` folder will not preserve the required build order.
- **Probing a NonSecure breakpoint before Secure handoff**: if the NS image does not run, debug the Secure image first to verify clock, GTZC, and vector-table handoff.

## Typical Performance Baseline (Articles 05--15)

For the reproducible 160 MHz / SMPS / VOS1 / two-way ICACHE configuration,
`Performance` build preset, ownership boundaries and real-board validation record,
see [docs/PERFORMANCE_BASELINE_05_15.md](docs/PERFORMANCE_BASELINE_05_15.md).
It deliberately separates this CPU/bus performance baseline and the measured
TIM2/GPDMA ADC/DAC streams from unmeasured Stop/Standby current work and PA4
scope/DMM analog measurements.

## Current Demo Mapping

| Article | Build value | Code file | Status (2026-07-12) |
| --- | --- | --- | --- |
| 03 | `U585_ACTIVE_DEMO=3` | `exp03_led_button.c` | Measured: LED/VCP OK |
| 04 | `4` | `exp04_debug_fault.c` | Measured: VCP+ITM probe; fault macro off |
| 05 | `5` | `exp05_clock_tree.c` | Measured: 160 MHz / latency 4 |
| 06 | `6` | `exp06_power_supply.c` | Measured: SMPS+VOS1 registers |
| 07 | `7` | `exp07_low_power_current.c` | Sleep hook OK; Stop/Standby author DMM |
| 08 | `8` | `exp08_rtc_wakeup_backup.c` | Measured: NS RTC + backup DR0 |
| 09 | `9` | `exp09_gpio_exti.c` | EXTI enabled; press counts author |
| 10 | `10` | `exp10_tim_pwm_input_capture.c` | Measured: TIM2 IRQ software PWM |
| 11 | `11` | `exp11_uart_printf_log.c` | Measured: NS USART1 printf |
| 12 | `12` | `exp12_i2c_sensor_bus.c` | Measured: I2C2 scan found 7 addrs |
| 13 | `13` | `exp13_spi_bus.c` | Measured: SPI2 Mode0 NS xfer_ok=1 |
| 14 | `14` | `exp14_adc_dma.c` | Measured: TIM2 TRGO -> ADC1 VREFINT -> GPDMA1 CH1 circular; VCP HT/TC counts progress |
| 15 | `15` | `exp15_dac_output.c` | Measured: TIM2 TRGO -> DAC1 PA4 stepped waveform via GPDMA1 CH2 circular; VCP HT/TC + DOR progress; scope/DMM pending |
| 16 | `16` | `exp16_gpdma_transfer.c` | Measured: GPDMA1 CH0 mem2mem mismatch=0 |
| 17 | `17` | `exp17_hts221_sensor.c` | Measured: WHO_AM_I=0xBC |
| 18 | `18` | `exp18_lps22hh_sensor.c` | Measured: LPS22HH WHO_AM_I=0xB3 |
| 19 | `19` | `exp19_ism330dhcx_imu.c` | Measured: ISM330DHCX WHO_AM_I=0x6B |
| 20 | `20` | `exp20_iis2mdc_compass.c` | Measured: IIS2MDC WHO_AM_I=0x40 |
| 21 | `21` | `exp21_vl53l5cx_tof.c` | Measured: LPn PH1 + I2C 0x29 probe/alive; ULD pending |
| 22 | `22` | `exp22_pdm_microphone.c` | Measured: ADF1 MIC1 poll sample_ok=1 |
| 23 | `23` | `exp23_ospi_flash_xip.c` | Measured: OCTOSPI2 JEDEC C2/85/3A (MX25LM51245G) |
| 24 | `24` | `exp24_ospi_psram_cache.c` | Measured: OCTOSPI1 init ready=1; SPI RW/ID pending |
| 25 | `25` | `exp25_st25dv_nfc.c` | Measured: no ST25DV; M24256 EEPROM @0x56 rw_ok=1 |
| 26 | `26` | `exp26_usb_ucpd_device.c` | Measured: UCPD CC sense ucpd_ready=1 |
| 27 | `27` | `exp27_wifi_emw3080.c` | Measured: Chip_En/SPI; flow_ok=0 |
| 28 | `28` | `exp28_mqtt_cloud.c` | Measured: offline CONNECT frame; Wi-Fi blocked |
| 29 | `29` | `exp29_icache_fetch.c` | Measured: ICACHE on/off ~1.8× |
| 30 | `30` | `exp30_rng_aes_pka.c` | Measured: RNG/AES/PKA ok |
| 31 | `31` | `exp31_trustzone_gtzc.c` | Measured: NSC + dual-image map |
| 32 | `32` | `exp32_tfm_secure_boot.c` | Measured: TF-M/SBSFU inventory stub |
| 33 | `33` | `exp33_ospi_otfdec.c` | Measured: OSPI PSRAM+Flash OTFDEC all_ok=1 |

Default demo is `3`. Logging prefers NonSecure USART1; `SECURE_UART1_WriteString` remains as NSC fallback if NS UART is not ready.

## Build / Flash / Capture

```powershell
cmake --preset Debug -DU585_ACTIVE_DEMO=5
cmake --build --preset Debug
powershell -ExecutionPolicy Bypass -File tools/flash_and_capture.ps1 -Demo 5 -Seconds 6
```

Root `CMakeLists.txt` forwards `U585_ACTIVE_DEMO` into the NonSecure ExternalProject.

## Hardware Notes

- red LED `PH6`, green LED `PH7`, user button `PC13`
- Button pressed level: `U585_BUTTON_PRESSED_STATE` (default `GPIO_PIN_SET`)
- Fault trigger (04): `U585_EXP04_ENABLE_FAULT_TRIGGER=1`
- Serial: ST-LINK VCP **COM3**, `115200 8N1`

## Measured data

See `docs/superpowers/measured/2026-07-12-phase-ab-summary.md` and `demoXX-com3.txt`.

## Verification

```powershell
python tools/check_experiment_layout.py
```
