# U585 Phase A/B Measured Capture Summary

Date: 2026-07-12  
Board: B-U585I-IOT02A (ST-LINK SN 001E003B4D46501220383832)  
VCP: COM3 @ 115200 8N1  
Host capture: `tools/flash_and_capture.ps1`

## Machine-captured (usable in articles)

| Demo | Key results |
|------|-------------|
| 03 | LED/button VCP OK; `initial_button=0`; LED alternate `led_state=1/2`; heartbeat |
| 04 | Start banner OK; `itm_trcena=0` without debugger TRCENA; `fault_trigger_enabled=0` |
| 05 | SYSCLK/HCLK/PCLK1/2/3 = **160000000**; FLASH_LATENCY=**4** |
| 06 | `smps_selected=1`; `voltage_range=196608` (VOS1); `PWR_CR3=2` |
| 07 | Sleep hook enabled; button->Sleep deferred for author press test |
| 08 | `rtc_ready=1`; `backup0=0xA5850008`; seconds increment |
| 09 | `exti_enabled=1`; button EXTI counts need author press |
| 10 | `tim_ready=1`; TIM2 IRQ to NS; software PWM duty default 20% |
| 11 | `uart_ready=1`; NonSecure USART1 printf heartbeats on VCP |

## Author follow-up (screenshots / instruments)

- CubeMX/VS Code screenshots (03)
- SWO/ITM host window + Fault handler screenshots (04)
- Current measurement on JP path for SMPS/LDO/Sleep/Stop/Standby (06/07)
- RTC wakeup from Stop/Standby current (08)
- Button press EXTI log + photo (09)
- Scope PWM waveform + input-capture (10)
- Serial terminal screenshot for publishing (11)

Raw logs: `docs/superpowers/measured/demoXX-com3.txt`

## Phase C addendum (I2C)

| Demo | Key results |
|------|-------------|
| 12 | I2C2 NS scan: 7 devices (0x10,0x1E,0x56,0x5D,0x5E,0x5F,0x6B) |
| 17 | HTS221 WHO_AM_I=0xBC ok; raw_hum/raw_temp streaming |
| 18 | LPS22HH WHO_AM_I=0xB3 ok; raw_press/raw_temp streaming |
| 19 | ISM330DHCX WHO_AM_I=0x6B ok; ax/ay/az/gx streaming |
| 20 | IIS2MDC WHO_AM_I=0x40 ok; mx/my/mz streaming |

## Phase D addendum (SPI / analog / DMA)

| Demo | Key results |
|------|-------------|
| 13 | SPI2 NS Mode0 8-bit soft-NSS; `spi_ready=1` `xfer_ok=1`; tx0=0x9F, rx=[11,0,0,255] on WRLS path |
| 14 | ADC1 NS polling VREFINT/TEMP; `adc_ready=1`; vrefint_raw≈1482; vdda_mv≈3348; dma_used=0 |
| 16 | GPDMA1 CH0 NS mem2mem; `xfer_ok=1` `mismatch=0` (channel PRIV required by SRAM3 MPCBB) |

