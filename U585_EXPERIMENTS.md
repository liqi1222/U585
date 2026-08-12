# U585 实验代码布局

本项目将面向文章的测试代码放在 NonSecure 应用层，除非某个实验明确与 TrustZone 相关。

## 默认边界

- `Secure/` 负责 TrustZone 启动交接、系统时钟/电源配置、GTZC 初始配置，以及通过 USART1 输出 Secure 启动横幅。
- Secure 横幅输出后，Secure 会将 USART1 / TIM2 / RTC / I2C2（演示需要时还包括 SPI2）交给 NonSecure（GTZC NSEC），释放相关 GPIO 安全位，清除 EXTI13 安全位，并通过 `NVIC_SetTargetState` 将演示用中断路由到 NS。
- `NonSecure/App/` 负责文章演示、开发板辅助函数、NS USART1（`u585_usart1.c`）以及 VCP 日志（`u585_log.c` 在 NS UART 就绪后使用 NS UART，否则回退到 NSC）。
- 第 31 和第 32 篇仍属于 Secure/TF-M 主题。
- 双镜像布局：`TZEN=1`，`SECBOOTADD0=0x0C000000`，Bank2 NS 位于 `0x08100000`。已验证的 OB：`SECWM2_PSTRT=0x7F SECWM2_PEND=0x0`。
- LED PH6/PH7 为 NonSecure（`GPIOH->SECCFGR` 清除 SEC6/SEC7）。

## 项目基线（第 03 篇）

第 03 篇建立了后续所有实验使用的 **CubeMX + VS Code + CMake** 基线。目标是形成一个最小且可复现的闭环：配置 → 生成 → 构建 → 下载 → 调试 → 运行。

### CubeMX 最小配置

将 `U585.ioc` 作为唯一事实来源。基线项目目标如下：

- **MCU**：STM32U585AIIxQ（UFBGA169）
- **TrustZone**：已启用（`Mcu.ContextProject=TrustZoneEnabled`）
- **工具链**：CMake（`ProjectManager.TargetToolchain=CMake`）
- **固件包**：STM32Cube FW_U5 V1.8.0

对于第一个 LED/按键演示，只需要配置以下三个用户可见项目：

| 项目 | 设置 | 说明 |
| --- | --- | --- |
| 时钟 | 由 PLL 提供 `SYSCLK = 160 MHz`（MSI 4 MHz，M=1/N=80/R=2） | 保守且稳定；第 05 篇将探索更高/更低的配置。 |
| 红色 LED | `PH6` = `GPIO_Output`，标签为 `LED_RED`，上拉，高速 | NonSecure 引脚（`CortexM33NS`）。 |
| 绿色 LED | `PH7` = `GPIO_Output`，标签为 `LED_GREEN`，上拉，高速 | NonSecure 引脚（`CortexM33NS`）。 |
| 用户按键 | `PC13` = `GPIO_Input`，标签为 `USER_Button`，无上下拉 | NonSecure 引脚；在本开发板上，未按下时读取值为 `0`。 |

将 `.ioc` 文件纳入 Git 管理。每次修改 `.ioc` 后都要重新生成代码；手写应用代码应放在 `USER CODE BEGIN/END` 区域或 `NonSecure/App/` 中，以免被重新生成覆盖。

### VS Code / CMake / 调试设置

工作区使用第 03 篇所述的 **分离式工具链**：

1. **构建**：通过 `gcc-arm-none-eabi.cmake` 使用 CMake + Ninja + `arm-none-eabi-gcc`。
2. **下载**：STM32CubeProgrammer（ST-LINK）或 OpenOCD。
3. **调试**：VS Code 的 `cortex-debug` 扩展 + ST-LINK GDB 服务器。

打开 `U585.code-workspace`（不要打开子文件夹），这样根目录预设会先构建 `Secure`，再构建 `NonSecure`：

```powershell
cmake --preset Debug
cmake --build --preset Debug
```

对于基线演示，保持 `U585_ACTIVE_DEMO=3`（根目录 `CMakeLists.txt` 中的默认值）。按 F5 会启动 **U585 NonSecure (ST-LINK)**，烧录两个 TrustZone 镜像，并在 `NonSecure/Core/Src/main.c:main` 处暂停。板载 ST-LINK/V3E 通过 `1000 kHz` SWD 负责下载和调试。扩展列表、路径覆盖和故障排查请参阅 [docs/VS_CODE.md](docs/VS_CODE.md)。

### LED/按键演示行为

`NonSecure/App/Src/exp03_led_button.c`：

- 初始状态：红色 LED 亮，绿色 LED 灭。
- 每次循环交换两个 LED 的状态。
- 默认闪烁周期：500 ms。
- 按下用户按键后，在 500 ms 和 100 ms 之间切换周期（边沿检测）。
- VCP 输出 `led_state=`、`heartbeat=` 和 `button_edge_delay_ms=`。

### 第 03 篇实测记录

**2026-07-12（原文章记录）**

- **开发板**：B-U585I-IOT02A，ST-LINK VCP **COM3 @ 115200 8N1**。
- **构建**：`U585_ACTIVE_DEMO=3`，默认 Debug 预设。
- **下载**：STM32CubeProgrammer + VS Code `cortex-debug`（ST-LINK 路径；未使用 OpenOCD）。
- **现象**：红绿 LED 交替闪烁；用户按键边沿会改变闪烁速率；VCP 输出 `[U585][03] led_state=` 和 `heartbeat=` 行。
- **原始日志**：`docs/superpowers/measured/demo03-com3.txt`。

**2026-08-07（从本工作区重新验证）**

- **工具链**：CMake 3.28.1 + Ninja 1.11.1 + arm-none-eabi-gcc 14.3.1（STM32CubeCLT 1.21.0）。
- **构建**：Debug 预设的全新配置/构建成功（`U585_S.elf` 45.95 KB @ `0x0C000000`，`U585_NS.elf` 21.02 KB @ `0x08100000`）。
- **调试探针**：ST-LINK/V3E SN `001E003B4D46501220383832`，固件 V3J17M10，SWD 1000 kHz。
- **下载**：使用 `STM32_Programmer_CLI.exe` 烧录并验证两个镜像。
- **运行捕获（COM3，6 秒）**：

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

- **状态**：✅ LED/VCP 循环运行；已确认从本次检出版本执行下载的路径。

### 常见基线问题

- **直接编辑生成文件**：始终修改 `.ioc` 后重新生成，或将代码放在 `USER CODE` 标记区域内。
- **忘记将 `.ioc` 纳入 Git**：没有该文件，就无法在另一台机器上复现配置。
- **打开子文件夹而不是工作区**：独立打开 `NonSecure` 或 `Secure` 文件夹无法保留所需的构建顺序。
- **在 Secure 交接前设置 NonSecure 断点**：如果 NS 镜像没有运行，应先调试 Secure 镜像，确认时钟、GTZC 和向量表交接正常。

## 典型性能基线（第 05--15 篇）

关于可复现的 160 MHz / SMPS / VOS1 / 双向 ICACHE 配置、`Performance` 构建预设、所有权边界和真实开发板验证记录，请参阅 [docs/PERFORMANCE_BASELINE_05_15.md](docs/PERFORMANCE_BASELINE_05_15.md)。
该文档明确区分了 CPU/总线性能基线及已实测的 TIM2/GPDMA ADC/DAC 数据流，也区分了尚未实测的 Stop/Standby 电流工作和 PA4 示波器/DMM 模拟测量。

## 当前演示映射

| 文章 | 构建值 | 代码文件 | 状态（2026-07-12） |
| --- | --- | --- | --- |
| 03 | `U585_ACTIVE_DEMO=3` | `exp03_led_button.c` | 已实测：LED/VCP 正常 |
| 04 | `4` | `exp04_debug_fault.c` | 已实测：VCP+ITM 探针；故障宏关闭 |
| 05 | `5` | `exp05_clock_tree.c` | 已实测：160 MHz / 延迟 4 |
| 06 | `6` | `exp06_power_supply.c` | 已实测：SMPS+VOS1 寄存器 |
| 07 | `7` | `exp07_low_power_current.c` | Sleep 钩子正常；Stop/Standby 由作者使用 DMM 测量 |
| 08 | `8` | `exp08_rtc_wakeup_backup.c` | 已实测：NS RTC + 备份 DR0 |
| 09 | `9` | `exp09_gpio_exti.c` | EXTI 已启用；按键次数由作者测量 |
| 10 | `10` | `exp10_tim_pwm_input_capture.c` | 已实测：TIM2 中断软件 PWM |
| 11 | `11` | `exp11_uart_printf_log.c` | 已实测：NS USART1 printf |
| 12 | `12` | `exp12_i2c_sensor_bus.c` | 已实测：I2C2 扫描发现 7 个地址 |
| 13 | `13` | `exp13_spi_bus.c` | 已实测：SPI2 Mode0 NS 传输成功标志为 1 |
| 14 | `14` | `exp14_adc_dma.c` | 已实测：TIM2 TRGO → ADC1 VREFINT → GPDMA1 CH1 环形传输；VCP HT/TC 计数持续增长 |
| 15 | `15` | `exp15_dac_output.c` | 已实测：TIM2 TRGO → 通过 GPDMA1 CH2 环形传输驱动 DAC1 PA4 阶梯波形（PA4 stepped waveform）；VCP HT/TC + DOR 持续增长；待示波器/DMM 验证 |
| 16 | `16` | `exp16_gpdma_transfer.c` | 已实测：GPDMA1 CH0 存储器到存储器传输 mismatch=0 |
| 17 | `17` | `exp17_hts221_sensor.c` | 已实测：WHO_AM_I=0xBC |
| 18 | `18` | `exp18_lps22hh_sensor.c` | 已实测：LPS22HH WHO_AM_I=0xB3 |
| 19 | `19` | `exp19_ism330dhcx_imu.c` | 已实测：ISM330DHCX WHO_AM_I=0x6B |
| 20 | `20` | `exp20_iis2mdc_compass.c` | 已实测：IIS2MDC WHO_AM_I=0x40 |
| 21 | `21` | `exp21_vl53l5cx_tof.c` | 已实测：LPn PH1 + I2C 0x29 探测/存活；ULD 待验证 |
| 22 | `22` | `exp22_pdm_microphone.c` | 已实测：ADF1 MIC1 轮询 sample_ok=1 |
| 23 | `23` | `exp23_ospi_flash_xip.c` | 已实测：OCTOSPI2 JEDEC C2/85/3A（MX25LM51245G） |
| 24 | `24` | `exp24_ospi_psram_cache.c` | 已实测：OCTOSPI1 初始化 ready=1；SPI 读写/ID 待验证 |
| 25 | `25` | `exp25_st25dv_nfc.c` | 已实测：未发现 ST25DV；M24256 EEPROM @0x56 rw_ok=1 |
| 26 | `26` | `exp26_usb_ucpd_device.c` | 已实测：UCPD CC 检测 ucpd_ready=1 |
| 27 | `27` | `exp27_wifi_emw3080.c` | 已实测：Chip_En/SPI；flow_ok=0 |
| 28 | `28` | `exp28_mqtt_cloud.c` | 已实测：离线 CONNECT 帧；Wi-Fi 受阻 |
| 29 | `29` | `exp29_icache_fetch.c` | 复测：ICACHE 命中计数增长；本次 DWT 计时窗口无效（`cycles_off=1`,`cycles_on=1`），性能倍数待修正后再下结论 |
| 30 | `30` | `exp30_rng_aes_pka.c` | 已实测：RNG/AES/PKA 正常 |
| 31 | `31` | `exp31_trustzone_gtzc.c` | 已实测：NSC + 双镜像映射 |
| 32 | `32` | `exp32_tfm_secure_boot.c` | 已实测：TF-M/SBSFU 清单存根 |
| 33 | `33` | `exp33_ospi_otfdec.c` | 已实测：OSPI PSRAM+Flash OTFDEC all_ok=1 |

默认演示为 `3`。日志优先使用 NonSecure USART1；如果 NS UART 尚未就绪，则保留 `SECURE_UART1_WriteString` 作为 NSC 回退路径。

## 构建 / 烧录 / 捕获

```powershell
cmake --preset Debug -DU585_ACTIVE_DEMO=5
cmake --build --preset Debug
powershell -ExecutionPolicy Bypass -File tools/flash_and_capture.ps1 -Demo 5 -Seconds 6
```

根目录 `CMakeLists.txt` 会将 `U585_ACTIVE_DEMO` 转发给 NonSecure ExternalProject。

## 硬件说明

- 红色 LED：`PH6`；绿色 LED：`PH7`；用户按键：`PC13`
- 按键按下电平：`U585_BUTTON_PRESSED_STATE`（默认值为 `GPIO_PIN_SET`）
- 故障触发（04）：`U585_EXP04_ENABLE_FAULT_TRIGGER=1`
- 串口：ST-LINK VCP **COM3**，`115200 8N1`

## 实测数据

请参阅 `docs/superpowers/measured/2026-07-12-phase-ab-summary.md` 和 `demoXX-com3.txt`。

## 验证

```powershell
python tools/check_experiment_layout.py
```
