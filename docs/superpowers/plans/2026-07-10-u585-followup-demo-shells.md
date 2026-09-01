# U585 Follow-up Demo Shells Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Extend the U585 NonSecure experiment framework so articles 06-31 each have an independent, selectable, build-verified code entry.

**Architecture:** Keep the existing `NonSecure/App` facade and dispatcher. Add one focused source file per article, each exporting a `U585_Demo` object and a small `volatile` state block for debugger inspection. Keep TrustZone-specific articles 32 and 33 out of the default NonSecure demo range.

**Tech Stack:** STM32CubeMX CMake project, ARM GCC, STM32 HAL, Python structural verification script.

---

### Task 1: Expand Structural Verification

**Files:**
- Modify: `tools/check_experiment_layout.py`

- [x] **Step 1: Require demo files 06-31**

Add a `DEMO_SOURCES` map covering 03-31 and make the check require each `NonSecure/App/Src/expNN_*.c` file.

- [x] **Step 2: Watch the check fail before implementation**

Run: `python tools/check_experiment_layout.py`

Expected before implementation: FAIL listing missing `exp06_...` through `exp31_...` files.

### Task 2: Add Follow-up Demo Sources

**Files:**
- Create: `NonSecure/App/Src/exp06_power_supply.c`
- Create: `NonSecure/App/Src/exp07_low_power_current.c`
- Create: `NonSecure/App/Src/exp08_rtc_wakeup_backup.c`
- Create: `NonSecure/App/Src/exp09_gpio_exti.c`
- Create: `NonSecure/App/Src/exp10_tim_pwm_input_capture.c`
- Create: `NonSecure/App/Src/exp11_uart_printf_log.c`
- Create: `NonSecure/App/Src/exp12_i2c_sensor_bus.c`
- Create: `NonSecure/App/Src/exp13_spi_bus.c`
- Create: `NonSecure/App/Src/exp14_adc_dma.c`
- Create: `NonSecure/App/Src/exp15_dac_output.c`
- Create: `NonSecure/App/Src/exp16_gpdma_transfer.c`
- Create: `NonSecure/App/Src/exp17_hts221_sensor.c`
- Create: `NonSecure/App/Src/exp18_lps22hh_sensor.c`
- Create: `NonSecure/App/Src/exp19_ism330dhcx_imu.c`
- Create: `NonSecure/App/Src/exp20_iis2mdc_compass.c`
- Create: `NonSecure/App/Src/exp21_vl53l5cx_tof.c`
- Create: `NonSecure/App/Src/exp22_pdm_microphone.c`
- Create: `NonSecure/App/Src/exp23_ospi_flash_xip.c`
- Create: `NonSecure/App/Src/exp24_ospi_psram_cache.c`
- Create: `NonSecure/App/Src/exp25_st25dv_nfc.c`
- Create: `NonSecure/App/Src/exp26_usb_ucpd_device.c`
- Create: `NonSecure/App/Src/exp27_wifi_emw3080.c`
- ~~Create: `NonSecure/App/Src/exp28_freertos_queue_log.c`~~ **Removed from plan** (FreeRTOS = pure software; not chip/board capability)
- Create: `NonSecure/App/Src/exp28_mqtt_cloud.c` (was planned as 29; FreeRTOS dropped)
- Create: `NonSecure/App/Src/exp29_icache_fetch.c`
- Create: `NonSecure/App/Src/exp30_rng_aes_pka.c`

- [x] **Step 1: Add compile-safe article shells**

Each file exports a `U585_Demo_ExpNN` object, initializes basic GPIO, updates a `volatile` debug state, and keeps hardware-specific behavior behind a future per-article migration point.

### Task 3: Wire Dispatcher and Build System

**Files:**
- Modify: `NonSecure/App/Inc/u585_demo.h`
- Modify: `NonSecure/App/Src/u585_app.c`
- Modify: `NonSecure/CMakeLists.txt`

- [x] **Step 1: Declare demo objects**

Add `extern const U585_Demo U585_Demo_Exp06` through `U585_Demo_Exp32`.

- [x] **Step 2: Extend dispatcher**

Add `U585_ACTIVE_DEMO == 6` through `31` branches.

- [x] **Step 3: Extend CMake**

Add sources 06-31 and expose the cache string values `3` through `31`.

### Task 4: Update Documentation

**Files:**
- Modify: `U585_EXPERIMENTS.md`

- [x] **Step 1: Add the 06-31 mapping table**

Document the build value, file name, and purpose for each article demo.

- [x] **Step 2: Note shell status and TrustZone boundary**

Explain that many 06-31 entries are compile-safe experiment shells until the matching peripheral is moved or exposed to NonSecure.

### Task 5: Verify

**Files:**
- Run-only verification

- [x] **Step 1: Run structural check**

Run: `python tools/check_experiment_layout.py`

Expected: PASS.

- [x] **Step 2: Build root Debug preset**

Run: `cmake --preset Debug` and `cmake --build --preset Debug`

Expected: Secure and NonSecure build successfully.

- [x] **Step 3: Build representative NonSecure selections**

Run direct builds for `U585_ACTIVE_DEMO=6`, `17`, and `31`.

Expected: all representative selections link successfully.
