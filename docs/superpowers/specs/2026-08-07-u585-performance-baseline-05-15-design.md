# U585 05--15 Typical Performance Baseline Design

## Goal

Make the B-U585I-IOT02A project reproducibly build and run with the safe, typical
high-performance baseline established by articles 05--15, while keeping low-power
and unmeasured DMA paths explicitly optional.

## Selected approach

Use the existing TrustZone two-image architecture and make its current 160 MHz
configuration explicit rather than creating a second clock tree.  The profile is
SMPS, VOS1, MSI 4 MHz -> PLL (M=1, N=80, R=2), 160 MHz on SYSCLK/HCLK/PCLK1/2/3,
Flash latency 4, and enabled two-way instruction cache.  This is the lowest-risk
choice because it is already the measured clock/power configuration of this board.

The CMake `Performance` preset will use `-O2 -g3 -DNDEBUG`.  `-O2` is selected over
`-O3`: it is a portable performance default with debuggable symbols and avoids
claiming a workload-specific gain that has not been measured on this application.

## Boundaries

- CubeMX remains the owner of clock, PWR, ICACHE and pin/peripheral declarations;
  the `.ioc` and generated Secure code must agree.
- Secure boot retains RCC/PWR ownership, then hands USART1, TIM2, RTC, I2C2, SPI2,
  ADC12, DAC1 and GPDMA1 to NonSecure exactly as the current handoff does.
- Stop/Standby and RTC LSE remain outside this performance profile.  Demo 14 and
  Demo 15 use the repeatable streaming paths required by their articles: TIM2 update
  TRGO at 10 kHz, ADC1 VREFINT or DAC1_OUT1 respectively, and a one-node GPDMA
  linked-list queue in circular mode.  ADC uses GPDMA1 Channel 1 and DAC uses Channel
  2 so the existing Channel-0 memory-copy experiment stays independent.  The two
  demos are separately selected at build time, so their shared timer never runs
  concurrently.
- PA4 remains DAC1_OUT1 and is electrically shared with STMod+ SPI1_NSS.  The DAC
  wave demonstration must therefore be used with SPI1/STMod+ disconnected; it does
  not assert a DAC-to-ADC loopback without an external jumper and measurement.
- DMA guidance must not mention D-cache maintenance: STM32U585 has an ICACHE, not a
  CPU data cache.  DMA buffer ownership/alignment and cacheability requirements are
  to be checked again if an external cache/memory mapped accelerator is introduced.

## Verification

`tools/check_experiment_layout.py` asserts the profile declarations, timer/DMA
topology, TrustZone IRQ handoff and the observability fields.  A fresh `Performance`
configure and build validates both TrustZone images.  Demos 14 and 15 are each
flashed with a VCP capture that must report a nonzero DMA half/full-transfer count.
