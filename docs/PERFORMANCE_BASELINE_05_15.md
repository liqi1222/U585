# B-U585I-IOT02A 05--15 典型性能基线

## 14--15 定时器触发 GPDMA 流（已实现）

本节取代本文前面“ADC/DAC 流式 DMA 待实现”的旧状态描述。两个演示均在
`Performance` 配置下独占运行：它们共用 **TIM2**，但一次只会编译一个
`U585_ACTIVE_DEMO`，因此不会发生计时器资源竞争。

| 演示 | 触发与数据路径 | 可观察结果 |
| --- | --- | --- |
| 14 | TIM2 update TRGO = 10 kHz -> ADC1 VREFINT -> GPDMA1 Channel 1 -> 128 个 `uint16_t` SRAM 缓冲区 | VCP 输出 `dma_half_count`、`dma_full_count`、最新 `vrefint_raw` 与推算 `vdda_mv`。 |
| 15 | TIM2 update TRGO = 10 kHz -> DAC1 Channel 1 / PA4；GPDMA1 Channel 2 从 128 点 12-bit `uint32_t` 三角表以 word 写入 DHR12R1 | VCP 输出 DMA 半满/全满次数和 DOR；输出频率约 78 Hz 的 128 阶梯波，可用示波器测量。 |

STM32U5 的 GPDMA 不能把传统 `DMA_CIRCULAR` 当作循环传输。本工程使用
**linked-list circular**：每条数据流有一个对齐的静态节点和队列，并依次调用
`HAL_DMAEx_List_BuildNode`、`HAL_DMAEx_List_SetCircularMode` 与
`HAL_DMAEx_List_LinkQ`。链表模式可让 HAL 在 `HAL_ADC_Start_DMA` 或
`HAL_DAC_Start_DMA` 启动时填入本次传输的地址和长度。半满/全满回调只递增状态
计数；串口日志留在主循环，避免干扰 10 kHz 数据路径。

Demo 14 的 `VREFINT_CAL` 是芯片在 3.0 V 下以 ADC1 14-bit 分辨率写入的工厂值。
因此 12-bit DMA 原始值先左移两位再计算 `VDDA = VREFINT_CAL_VREF * VREFINT_CAL /
(raw12 << 2)`；这与 STM32U5 LL 的换算口径一致。VCP 的 `vdda_mv` 是该工厂校准推导值，
不是用 1212 mV 典型值的估算。

ADC 的 DR 与采样缓冲区使用 halfword 传输；DAC 的 DHR12R1 数据寄存器使用 word
传输和 `uint32_t` 查表，这是 ST 的 STM32U5 DAC-DMA 参考工程采用的宽度。将 DAC
误设为 halfword 会导致首个请求无法被服务、触发 DMA underrun，并由 HAL 关闭 DMAEN。

Channel 0 仍由 Demo 16 的内存复制测试独占，故 ADC 和 DAC 分别使用 Channel 1
和 Channel 2。Secure 启动阶段为这两个通道设置 NonSecure、privileged、NonSecure
source/destination 属性，并把 ADC1、DAC1 和两个 GPDMA IRQ 目标交给 NonSecure。
这四层（GTZC 外设、DMA 通道、GPIO、IRQ target）缺少任何一层都会使流式演示失效。

PA4 是 `DAC1_OUT1`，同时也是 STMod+ 的 `SPI1_NSS`。运行 Demo 15 时不要连接或驱动
该 STMod+ SPI1 片选；本演示没有宣称存在板载 DAC-to-ADC 回环。若要验证模拟闭环，
应在确认电压范围不超过 VDDA 后，用跳线把 PA4 接至允许的 ADC 输入并另行记录测量。

分别验证两条流：

```powershell
powershell -ExecutionPolicy Bypass -File tools/flash_and_capture.ps1 -Demo 14 -Seconds 6
powershell -ExecutionPolicy Bypass -File tools/flash_and_capture.ps1 -Demo 15 -Seconds 6
```

### 2026-08-07 实板 DMA 记录

- Demo 14：`demo14-com3.txt` 中启动后 `dma_half_count=42`、`dma_full_count=42`，
  `vrefint_cal=6530`、`vrefint_raw=1482`、`vdda_mv=3304`；说明 10 kHz ADC1/GPDMA1 Channel 1
  循环流和回调均在运行。
- Demo 15：`demo15-com3.txt` 中启动后 `dma_half_count=42`、`dma_full_count=42`、
  `dor=1300`；说明 TIM2、DAC1 与 GPDMA1 Channel 2 的 word 数据流正在循环。
- 原始串口记录保存在 `docs/superpowers/measured/demo14-com3.txt` 和
  `docs/superpowers/measured/demo15-com3.txt`。DOR 只能证明寄存器更新；PA4 的模拟
  电压/阶梯波形仍应由示波器或万用表在未连接 STMod+ SPI1 的条件下独立确认。

只有在原始 VCP 文件显示 `dma_started=1` 且至少一个 DMA 半满或全满计数非零时，
才可把该演示记录为板端运行。对于 U585 本身不要加入 Cortex-M7 风格的 D-cache
clean/invalidate 说法：该基线启用了 ICACHE，而不是 CPU D-cache。

本文把实测笔记 05--15 中可以同时成立的设置收敛为一个可复现的开发档。
它优先保证 CPU、片上 Flash 取指和常用外设总线的吞吐；它不是低功耗档，并且把
已实板验证的 ADC/DAC 流式 DMA 与仍需示波器/万用表确认的模拟量测分开记录。

## 已采用的配置

| 层级 | 设置 | 工程位置 | 说明 |
| --- | --- | --- | --- |
| 供电 | `PWR_SMPS_SUPPLY` + VOS1/SCALE1 | `U585.ioc`、`Secure/Core/Src/main.c` | 这是本板已记录的 SMPS 工作路径；跳线或供电硬件改变时必须重新确认。 |
| 时钟 | MSI 4 MHz -> PLL `M=1,N=80,R=2` -> 160 MHz | `Secure/Core/Src/main.c` | `SYSCLK/HCLK/PCLK1/PCLK2/PCLK3` 均为 160 MHz，Flash latency 为 4。并非 HSE 16 MHz。 |
| 取指 | ICACHE enabled, two-way set-associative | `U585.ioc`、`Secure/Core/Src/icache.c` | `.ioc` 的 `DefaultMode` 对应 CubeMX 默认的 2-way 配置。 |
| 编译 | `Performance`: `-O2 -g3 -DNDEBUG` | `gcc-arm-none-eabi.cmake`、各 `CMakePresets.json` | `-O2` 是默认性能档，仍保留 ST-LINK 调试符号；先基准测试再考虑 `-O3`。 |
| 调试输出 | USART1 VCP, 115200 8N1 | Secure handoff + `NonSecure/App` | 只用于低频状态；ISR、高频采样和吞吐测试不能依赖阻塞日志。 |
| I2C | I2C2: PH4/PH5, open-drain, 外部上拉 | `U585.ioc`、`u585_i2c2.c` | 当前时序用于板载传感器；线长、上拉和总线电容变化后应复核。 |
| SPI | SPI2 Mode 0、8-bit、软件 NSS | `U585.ioc`、`u585_spi2.c` | PB12 由 NonSecure 软件拉片选；总线大块传输再单独引入 DMA。 |

## TrustZone 所有权

RCC、PWR、ICACHE 保持 Secure 启动期配置，避免 NonSecure 在系统运行时切换全局
时钟/电源状态。Secure 打印启动信息后移交 USART1、TIM2、RTC、I2C2、SPI2、ADC12、
DAC1 和 GPDMA1 及相关 GPIO/IRQ 给 NonSecure。新增外设时必须同时检查：CubeMX
安全属性、GTZC 外设属性、GPIO `SECCFGR`、DMA channel 的 source/destination 属性
以及 IRQ target state，不能只改其中一层。

## 构建、下载和观察

在工程根目录执行：

```powershell
cmake --preset Performance -DU585_ACTIVE_DEMO=5
cmake --build --preset Performance
powershell -ExecutionPolicy Bypass -File tools/flash_and_capture.ps1 -Demo 5 -Seconds 6
```

Demo 05 的 VCP 输出必须至少包含以下观察点：

```text
[U585][05] SystemCoreClock_Hz=160000000
[U585][05] FLASH_LATENCY=4
[U585][05] smps_selected=1
[U585][05] icache_enabled=1
[U585][05] icache_2ways=1
```

`voltage_range` 是 HAL 的枚举原值，用它与 SMPS、频率和 Flash latency 一并留档；
不要把某个枚举数值硬编码为跨版本结论。

### 2026-08-07 实板记录

- **构建**：`Performance`，`U585_ACTIVE_DEMO=5`，Secure 与 NonSecure 均由
  `arm-none-eabi-gcc 14.3.1` 构建，编译选项为 `-O2 -g3 -DNDEBUG`。
- **下载**：板载 ST-LINK/V3E，SN `001E003B4D46501220383832`，SWD 3300 kHz；
  STM32CubeProgrammer 2.22.0 写入 Secure `0x0C000000` 与 NonSecure `0x08100000`。
- **VCP**：COM3 / 115200 8N1，6 秒采集确认 `SystemCoreClock_Hz=160000000`、
  `FLASH_LATENCY=4`、`voltage_range=196608`、`smps_selected=1`、
  `icache_enabled=1`、`icache_2ways=1`。
- **原始记录**：`docs/superpowers/measured/demo05-com3.txt`。

## 05--15 的使用边界

- **低功耗（07--08）**：性能档不进入 Stop/Standby。要进入 Stop，必须在唤醒后恢复
  160 MHz 时钟；RTC 当前按 LSI 使用，LSE 需先通过实板确认。
- **GPIO/TIM（09--10）**：PC13 的 EXTI 和 TIM2 的中断/输出属于 NonSecure 试验资源。
  Demo 10 仍是软件 PWM 演示，不能作为高精度 PWM 结论；Demo 14/15 则已用同一 TIM2
  的内部 update TRGO 作 10 kHz 硬件采样/更新触发，且一次构建只运行其中一个演示。
- **ADC/DAC（14--15）**：Demo 14 已实现 ADC1 内部 VREFINT 的定时 GPDMA 循环采样，
  Demo 15 已实现 PA4 DAC1 的定时 GPDMA 循环查表输出；两者均有半满/全满 VCP 证据。
  PA4 与 STMod+ 的 `SPI1_NSS` 复用，接扩展板前先检查冲突；模拟电压与阶梯波形仍须
  用示波器或万用表独立测量。
- **DMA**：STM32U585 这里使用的是 ICACHE，并没有 CPU D-cache；不要照搬其他系列的
  clean/invalidate 步骤。DMA 缓冲需放在 NonSecure 可访问 SRAM、按传输宽度对齐，并对
  内存映射外设/外部存储的可缓存属性单独评审。

## 验证顺序

1. 构建并运行 `U585_ACTIVE_DEMO=5`，记录时钟、Flash、SMPS/VOS 和 ICACHE 行。
2. 依次运行 11、12、13、14、15，确认 VCP、I2C 扫描、SPI 回环、ADC 内部量和 DAC DOR。
3. 需要吞吐时，以 16 的 GPDMA 内存复制作为 DMA 访问权限基线；再运行 14/15，确认
   已保存的 TIM2/GPDMA 流式 DMA 半满/全满计数。模拟波形质量不由 VCP 代替测量。
4. 低功耗/IDD、真实 PWM/捕获波形、LSE 精度和 PA4 电压均单独测量并保存原始记录。
