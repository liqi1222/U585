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
