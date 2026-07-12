# U585 Experiment Code Layout

This project keeps article-facing test code under the NonSecure application layer unless an experiment is explicitly about TrustZone.

## Default Boundary

- `Secure/` keeps the current CubeMX-generated TrustZone boot handoff, system clock setup, power setup, GTZC setup, and Secure-side peripheral initialization.
- `NonSecure/Core/` keeps the generated NonSecure entry code. Hand-written article code should stay out of this folder except for small calls inside `USER CODE` sections.
- `NonSecure/App/` contains the reusable board helpers and independent article demos.
- Articles 32 and 33 are TrustZone/TFM topics and should be handled in `Secure/` or a dedicated secure experiment layer when they are implemented.
- The current VCP log path uses `SECURE_UART1_WriteString()` because USART1 is generated and initialized in the Secure context. NonSecure code calls it through `NonSecure/App/Src/u585_log.c`.
- The dual-image TrustZone layout requires `TZEN=1`, `SECBOOTADD0=0x0C000000`, Bank1 kept Secure, and Bank2 released for the NonSecure image at `0x08100000`. The verified Bank2 OB setting is `SECWM2_PSTRT=0x7F SECWM2_PEND=0x0`.
- LED GPIO ownership: LD6/LD7 are kept as NonSecure-controlled demo resources. Secure startup clears `GPIOH->SECCFGR` bits `SEC6` and `SEC7`, and the `.ioc` documents `PH6`/`PH7` as `CortexM33NS`; the NonSecure app then initializes and drives GPIOH pins directly.

## Current Demo Mapping

| Article | Build value | Code file | Purpose |
| --- | --- | --- | --- |
| 03 | `U585_ACTIVE_DEMO=3` | `NonSecure/App/Src/exp03_led_button.c` | CubeMX/VS Code baseline, LED blink, button polling |
| 04 | `U585_ACTIVE_DEMO=4` | `NonSecure/App/Src/exp04_debug_fault.c` | SWD/SWO practice and optional deliberate fault trigger |
| 05 | `U585_ACTIVE_DEMO=5` | `NonSecure/App/Src/exp05_clock_tree.c` | Clock-tree observation snapshot and LED heartbeat |
| 06 | `U585_ACTIVE_DEMO=6` | `NonSecure/App/Src/exp06_power_supply.c` | Power supply mode and voltage-scale measurement hook |
| 07 | `U585_ACTIVE_DEMO=7` | `NonSecure/App/Src/exp07_low_power_current.c` | Low-power/current-measurement hook |
| 08 | `U585_ACTIVE_DEMO=8` | `NonSecure/App/Src/exp08_rtc_wakeup_backup.c` | RTC wakeup and backup-domain shell |
| 09 | `U585_ACTIVE_DEMO=9` | `NonSecure/App/Src/exp09_gpio_exti.c` | GPIO polling baseline before EXTI migration |
| 10 | `U585_ACTIVE_DEMO=10` | `NonSecure/App/Src/exp10_tim_pwm_input_capture.c` | Timer PWM/input-capture shell |
| 11 | `U585_ACTIVE_DEMO=11` | `NonSecure/App/Src/exp11_uart_printf_log.c` | UART printf/log shell |
| 12 | `U585_ACTIVE_DEMO=12` | `NonSecure/App/Src/exp12_i2c_sensor_bus.c` | I2C sensor bus shell |
| 13 | `U585_ACTIVE_DEMO=13` | `NonSecure/App/Src/exp13_spi_bus.c` | SPI bus shell |
| 14 | `U585_ACTIVE_DEMO=14` | `NonSecure/App/Src/exp14_adc_dma.c` | ADC + DMA shell |
| 15 | `U585_ACTIVE_DEMO=15` | `NonSecure/App/Src/exp15_dac_output.c` | DAC output shell |
| 16 | `U585_ACTIVE_DEMO=16` | `NonSecure/App/Src/exp16_gpdma_transfer.c` | GPDMA transfer shell |
| 17 | `U585_ACTIVE_DEMO=17` | `NonSecure/App/Src/exp17_hts221_sensor.c` | HTS221 sensor shell |
| 18 | `U585_ACTIVE_DEMO=18` | `NonSecure/App/Src/exp18_lps22hh_sensor.c` | LPS22HH sensor shell |
| 19 | `U585_ACTIVE_DEMO=19` | `NonSecure/App/Src/exp19_ism330dhcx_imu.c` | ISM330DHCX IMU shell |
| 20 | `U585_ACTIVE_DEMO=20` | `NonSecure/App/Src/exp20_iis2mdc_compass.c` | IIS2MDC compass shell |
| 21 | `U585_ACTIVE_DEMO=21` | `NonSecure/App/Src/exp21_vl53l5cx_tof.c` | VL53L5CX ToF shell |
| 22 | `U585_ACTIVE_DEMO=22` | `NonSecure/App/Src/exp22_pdm_microphone.c` | PDM microphone shell |
| 23 | `U585_ACTIVE_DEMO=23` | `NonSecure/App/Src/exp23_ospi_flash_xip.c` | OctoSPI Flash/XIP shell |
| 24 | `U585_ACTIVE_DEMO=24` | `NonSecure/App/Src/exp24_ospi_psram_cache.c` | OctoSPI PSRAM shell |
| 25 | `U585_ACTIVE_DEMO=25` | `NonSecure/App/Src/exp25_st25dv_nfc.c` | ST25DV NFC shell |
| 26 | `U585_ACTIVE_DEMO=26` | `NonSecure/App/Src/exp26_usb_ucpd_device.c` | USB-C/UCPD shell |
| 27 | `U585_ACTIVE_DEMO=27` | `NonSecure/App/Src/exp27_wifi_emw3080.c` | EMW3080 Wi-Fi shell |
| 28 | `U585_ACTIVE_DEMO=28` | `NonSecure/App/Src/exp28_freertos_queue_log.c` | FreeRTOS queue/log shell |
| 29 | `U585_ACTIVE_DEMO=29` | `NonSecure/App/Src/exp29_mqtt_cloud.c` | MQTT cloud shell |
| 30 | `U585_ACTIVE_DEMO=30` | `NonSecure/App/Src/exp30_icache_fetch.c` | ICACHE observation shell |
| 31 | `U585_ACTIVE_DEMO=31` | `NonSecure/App/Src/exp31_rng_aes_pka.c` | RNG/AES/PKA shell |

The default value is `3`, so the root Debug preset builds the article 03 baseline unless `NonSecure` is configured directly with another value.

## Build

Root build, default demo 03:

```powershell
cmake --preset Debug
cmake --build --preset Debug
```

Direct NonSecure build for a specific demo:

```powershell
cmake -S NonSecure -B NonSecure/build/Debug-demo04 -G Ninja -DCMAKE_TOOLCHAIN_FILE=../gcc-arm-none-eabi.cmake -DCMAKE_BUILD_TYPE=Debug -DU585_ACTIVE_DEMO=4
cmake --build NonSecure/build/Debug-demo04
```

Use the same pattern with any value from `3` through `31`.

## Adding Later Articles

1. Add a focused source file under `NonSecure/App/Src`, for example `exp09_gpio_exti.c`.
2. Add its public demo object to `NonSecure/App/Inc/u585_demo.h`.
3. Add it to the dispatcher in `NonSecure/App/Src/u585_app.c`.
4. Add the source file to `NonSecure/CMakeLists.txt`.
5. Update this document with the article number, build value, and file name.

For non-TrustZone experiments, prefer moving only the needed peripheral access into the NonSecure side. Do not change Secure GTZC or CubeMX-generated files broadly just to make a future test compile.

The 06-31 files are intentionally independent and compile-safe. Many are currently experiment shells because the present `.ioc` places most peripherals in the Secure context. Fill each shell in article order after the needed peripheral has been made available to NonSecure code.

## Hardware Notes

The current board helper uses local aliases for:

- red LED: `PH6`
- green LED: `PH7`
- user button: `PC13`

The button pressed level defaults to `GPIO_PIN_SET` through `U585_BUTTON_PRESSED_STATE`. Confirm the actual pressed level on the board before publishing measured results. If the board behaves differently, override the macro in `NonSecure/CMakeLists.txt` or update `NonSecure/App/Inc/u585_board.h`.

The article 04 fault trigger is disabled by default. Enable it only while debugging:

```cmake
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE U585_EXP04_ENABLE_FAULT_TRIGGER=1)
```

## Serial Log

The board exposes ST-LINK VCP as `COM3` on this machine. Open it at `115200 8N1`.

Demo 03 prints startup lines, the initial button state, heartbeat counts, and button-edge delay changes. If the LED does not visibly blink, the serial log is the first check for whether the Secure boot handoff reached the NonSecure demo.

## Verification

Run the structural check after changing the experiment layout:

```powershell
python tools/check_experiment_layout.py
```
