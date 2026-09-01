# U585 05--15 Performance Baseline Implementation Plan

> Implementation record for the completed baseline work. The checked steps below map
> the contract, code, documentation, and board-capture evidence to the repository.

**Goal:** Provide a reproducible typical-performance configuration and an observable board validation path for articles 05--15.

**Architecture:** Retain the existing Secure-owned clock/power startup and NonSecure demo ownership.  Align CubeMX metadata and generated ICACHE code, add a custom CMake performance build type, and extend demo 05 so it reports the active profile directly from hardware registers.

**Tech Stack:** STM32CubeMX `.ioc`, STM32U5 HAL/CMSIS, CMake/Ninja, arm-none-eabi-gcc, Python validation.

---

### Task 1: Specify the profile contract

**Files:**
- Modify: `tools/check_experiment_layout.py`

- [x] **Step 1: Write the failing contract checks**

Add requirements for `PWR_SMPS_SUPPLY`, 160 MHz RCC values, `DefaultMode` ICACHE,
soft SPI2 NSS, the `Performance` CMake preset/toolchain flags, and demo-05 profile
fields.  Run `python tools/check_experiment_layout.py`; it must fail before the
implementation changes are made.

- [x] **Step 2: Implement only the declared configuration**

Change the CubeMX metadata, generated ICACHE selection and CMake toolchain/preset
files until the contract passes.

- [x] **Step 3: Run the contract check**

Run `python tools/check_experiment_layout.py` and expect `experiment layout check passed`.

### Task 2: Make the live profile observable

**Files:**
- Modify: `NonSecure/App/Src/exp05_clock_tree.c`

- [x] **Step 1: Use the contract check as the failing test**

Require `smps_selected`, `voltage_range`, `icache_enabled` and `icache_2ways` in
the demo source; run the Python check and expect a failure before adding them.

- [x] **Step 2: Capture registers and log them**

Add the four fields to the volatile snapshot.  Populate them from PWR/ICACHE and
write `[U585][05]` log lines alongside the existing frequency and Flash-latency
values.

- [x] **Step 3: Build both images**

Run `cmake --preset Performance -DU585_ACTIVE_DEMO=5` followed by
`cmake --build --preset Performance`; expect Secure and NonSecure ELF output.

### Task 3: Document use and limits

**Files:**
- Modify: `U585_EXPERIMENTS.md`
- Create: `docs/PERFORMANCE_BASELINE_05_15.md`

- [x] **Step 1: Record exact settings and commands**

Document the MSI-based PLL source (not HSE), SMPS/VOS1, Flash latency, two-way
ICACHE, `Performance` flags, GTZC ownership, I2C2/SPI2 settings and the flash/capture
command.

- [x] **Step 2: Separate measured facts from follow-up work**

State that RTC remains LSI until LSE is verified, Stop/Standby is not part of the
performance profile, and ADC/DAC streaming DMA requires a timer trigger plus a
configured request/channel before it can be claimed as implemented.

- [x] **Step 3: Verify documentation and code together**

Run the Python contract check, build the performance preset, flash demo 05, and
inspect the captured VCP lines.

### Task 4: Add the timer-paced, circular DMA contracts

**Files:**
- Modify: `tools/check_experiment_layout.py`
- Modify: `U585.ioc`
- Modify: `NonSecure/CMakeLists.txt`
- Create: `NonSecure/App/Inc/u585_tim2_trgo.h`
- Create: `NonSecure/App/Src/u585_tim2_trgo.c`
- Modify: `NonSecure/App/Inc/u585_adc1.h`
- Modify: `NonSecure/App/Src/u585_adc1.c`
- Modify: `NonSecure/App/Inc/u585_dac1.h`
- Modify: `NonSecure/App/Src/u585_dac1.c`
- Modify: `NonSecure/App/Inc/u585_gpdma.h`
- Modify: `NonSecure/App/Src/u585_gpdma.c`
- Modify: `Secure/Core/Src/main.c`
- Modify: `NonSecure/Core/Src/stm32u5xx_it.c`

- [x] **Step 1: Write and run the failing contract test**

Require a TIM2 update TRGO, ADC1/DAC1 timer trigger declarations, nonsecure
GPDMA Channels 1/2, `DMA_LINKEDLIST_CIRCULAR`, linked-list node APIs, peripheral
DMA starts, and TrustZone IRQ routing.  Run `python tools/check_experiment_layout.py`.
Expected: fail because the project still only contains polling/DC demos.

- [x] **Step 2: Implement the smallest reusable data path**

Add a 10 kHz TIM2 TRGO helper.  Configure GPDMA1 Channel 1 for ADC1 peripheral-to-
memory and Channel 2 for DAC1_CH1 memory-to-peripheral; each gets one static
linked-list node and queue built with `HAL_DMAEx_List_BuildNode`, circularized with
`HAL_DMAEx_List_SetCircularMode`, and linked with `HAL_DMAEx_List_LinkQ`.  Use
halfword transfers and leave Channel 0 unchanged.  Add the corresponding HAL link,
start and interrupt enable functions to ADC1/DAC1.  Release both channel resources
and all ADC/DAC/DMA IRQs to NonSecure in Secure startup, then route their handlers
to the HAL handles in NonSecure.

- [x] **Step 3: Run the contract test and compile Demo 14**

Run `python tools/check_experiment_layout.py`, then
`cmake --preset Performance -DU585_ACTIVE_DEMO=14` and `cmake --build --preset Performance`.
Expected: layout check passes and both TrustZone images build.

### Task 5: Turn articles 14 and 15 into observable streams

**Files:**
- Modify: `NonSecure/App/Src/exp10_tim_pwm_input_capture.c`
- Modify: `NonSecure/App/Src/exp14_adc_dma.c`
- Modify: `NonSecure/App/Src/exp15_dac_output.c`
- Modify: `docs/PERFORMANCE_BASELINE_05_15.md`

- [x] **Step 1: Start ADC DMA before TIM2 and count callbacks**

Use a 128-sample aligned VREFINT buffer.  Demo 14 shall call
`U585_ADC1_InitTimerDma`, `U585_ADC1_StartTimerDma`, then `U585_TIM2_TRGO_Start`.
Its ADC half/full callbacks only update volatile counters and its 500-ms VCP line
reports the latest VREFINT-derived VDDA plus those counters.

- [x] **Step 2: Start DAC DMA before TIM2 and count callbacks**

Use a 128-entry aligned 12-bit triangle table.  Demo 15 preloads the first DAC
value, calls `U585_DAC1_InitTimerDma`, `U585_DAC1_StartTimerDma`, then
`U585_TIM2_TRGO_Start`.  Its DAC half/full callbacks only update volatile counters
and VCP reports the running 10-kHz / 78-Hz stepped-wave configuration.  Keep the
PA4/STMod+ SPI1_NSS conflict warning explicit.

- [x] **Step 3: Build, flash and capture both demos**

Run the layout check, build and flash Demos 14 and 15 separately using the
`Performance` preset.  Store their raw VCP captures and require a nonzero half or
full callback count before documenting the result.

## Verification record

- `python tools/check_experiment_layout.py` -> `experiment layout check passed`.
- `cmake --preset Performance -DU585_ACTIVE_DEMO=5` and
  `cmake --build --preset Performance --clean-first --parallel 4` completed for
  both TrustZone images.
- `demo14-com3.txt` and `demo15-com3.txt` each record `dma_started=1` and
  increasing half/full callback counts.  The PA4 analog waveform remains an
  explicitly separate scope/DMM measurement.
