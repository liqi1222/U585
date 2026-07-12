# U585 Experiment Code Layout

This project keeps article-facing test code under the NonSecure application layer unless an experiment is explicitly about TrustZone.

## Default Boundary

- `Secure/` keeps TrustZone boot handoff, system clock/power setup, GTZC initial setup, and Secure boot banner on USART1.
- After the Secure banner, Secure **hands USART1 / TIM2 / RTC / I2C2 (/ SPI2 when demo needs it) to NonSecure** (GTZC NSEC), releases related GPIO security bits, clears EXTI13 secure bit, and routes demo IRQs to NS via `NVIC_SetTargetState`.
- `NonSecure/App/` owns article demos, board helpers, NS USART1 (`u585_usart1.c`), and VCP logging (`u585_log.c` uses NS UART when ready, NSC fallback otherwise).
- Articles 32 and 33 remain Secure/TF-M topics.
- Dual-image layout: `TZEN=1`, `SECBOOTADD0=0x0C000000`, Bank2 NS at `0x08100000`. Verified OB: `SECWM2_PSTRT=0x7F SECWM2_PEND=0x0`.
- LEDs PH6/PH7 are NonSecure (`GPIOH->SECCFGR` clears SEC6/SEC7).

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
| 14 | `14` | `exp14_adc_dma.c` | Measured: ADC1 VREFINT/TEMP poll; VDDA≈3.35V |
| 15 | `15` | `exp15_dac_output.c` | Shell |
| 16 | `16` | `exp16_gpdma_transfer.c` | Measured: GPDMA1 CH0 mem2mem mismatch=0 |
| 17 | `17` | `exp17_hts221_sensor.c` | Measured: WHO_AM_I=0xBC |
| 18 | `18` | `exp18_lps22hh_sensor.c` | Measured: LPS22HH WHO_AM_I=0xB3 |
| 19 | `19` | `exp19_ism330dhcx_imu.c` | Measured: ISM330DHCX WHO_AM_I=0x6B |
| 20 | `20` | `exp20_iis2mdc_compass.c` | Measured: IIS2MDC WHO_AM_I=0x40 |
| 21 | `21` | `exp21_vl53l5cx_tof.c` | Shell |
| 22 | `22` | `exp22_pdm_microphone.c` | Shell |
| 23 | `23` | `exp23_ospi_flash_xip.c` | Shell |
| 24 | `24` | `exp24_ospi_psram_cache.c` | Shell |
| 25 | `25` | `exp25_st25dv_nfc.c` | Shell |
| 26 | `26` | `exp26_usb_ucpd_device.c` | Shell |
| 27 | `27` | `exp27_wifi_emw3080.c` | Shell |
| 28 | `28` | `exp28_freertos_queue_log.c` | Shell |
| 29 | `29` | `exp29_mqtt_cloud.c` | Shell |
| 30 | `30` | `exp30_icache_fetch.c` | Shell |
| 31 | `31` | `exp31_rng_aes_pka.c` | Shell |
| 32–33 | Secure/TF-M | TrustZone topics | Out of NonSecure demo range |

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
