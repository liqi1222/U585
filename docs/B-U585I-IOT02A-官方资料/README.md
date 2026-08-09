# B-U585I-IOT02A 官方参考资料

> 为《B-U585I-IOT02A 实测笔记》系列准备的官方文档合集。
> 主控 MCU：**STM32U585AII6Q**（Cortex-M33）；板卡编号：**MB1551**。
> 来源：STMicroelectronics 官网（st.com），均为免费公开文档。
> 下载日期：2026-07-06

## 文档清单

| 文件 | 编号 | 用途 | 大小 | 来源 |
|------|------|------|------|------|
| UM2839-B-U585I-IOT02A-用户手册(含原理图).pdf | UM2839 | 板级用户手册：板载资源、引脚分配、跳线说明，附原理图概览 | ~3.1 MB | [下载](https://www.st.com/resource/en/user_manual/dm00773062-.pdf) |
| MB1551-U585I-C02-原理图.pdf | MB1551 Rev C02 | 独立板级原理图（16 页）：MCU IOs、OCTOSPI、STMOD+、传感器等 | ~8.7 MB | [下载](https://www.st.com/resource/en/schematic_pack/mb1551-u585i-c02_schematic.pdf) |
| RM0456-STM32U5系列-参考手册.pdf | RM0456 | STM32U5 系列参考手册：所有外设寄存器定义 | ~70 MB | [下载](https://www.st.com/resource/en/reference_manual/rm0456-stm32u5-series-armbased-32bit-mcus-stmicroelectronics.pdf) |
| DS13086-STM32U585xx-数据手册.pdf | DS13086 | STM32U585xx 数据手册：电气特性、封装、料号 | ~4.6 MB | [下载](https://www.st.com/resource/en/datasheet/stm32u585ai.pdf) |
| PM0264-Cortex-M33-编程手册.pdf | PM0264 | STM32 Cortex-M33 编程手册：内核寄存器（配合 ARMv8-M TRM） | ~11 MB | [下载](https://www.st.com/resource/en/programming_manual/pm0264-stm32-cortexm33-mcus-and-mpus-programming-manual-stmicroelectronics.pdf) |
| DB4410-B-U585I-IOT02A-数据简介.pdf | DB4410 | 板卡数据简介（Data Brief）：功能概览、卖点速查 | ~1.5 MB | [下载](https://www.st.com/resource/en/data_brief/b-u585i-iot02a.pdf) |

## 使用建议

- **入门顺序**：DB4410（速览）→ UM2839（板级细节 + 原理图）→ MB1551 原理图（查线路）→ RM0456 / DS13086（寄存器与电气特性按需查阅）。
- **写作对照**：正文里提到的 UM2839、RM0456、PM0264 已备齐；DS13086 即数据手册（正文标注的 DS13737 为 U575/U585 合订编号，此处为 U585 专版 DS13086，内容对应）。
- **配套固件**：STM32CubeU5 固件包体积大且随 CubeIDE 自动拉取，未纳入本目录，需要离线包时从 GitHub `STMicroelectronics/STM32CubeU5` 获取。

## 备注

- 全部文件下载后均已用 `%PDF-` 文件头校验为有效 PDF。
- ST 文档偶尔改版，编号后缀（Rev）可能更新；如需最新版可用上表链接复查。
