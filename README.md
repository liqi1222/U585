# U585 实测工程说明

B-U585I-IOT02A 开发板上的 **STM32U585AI**（Cortex-M33 + TrustZone）配套固件，对应公众号系列《B-U585I-IOT02A 实测笔记》demo **03–33**。

实验编号、引脚和实测状态以 [`U585_EXPERIMENTS.md`](U585_EXPERIMENTS.md) 为准。

GitHub 公开只读（仅本工程，不含手册 PDF）：https://github.com/liqi1222/U585  
他人 clone **`develop`**；只有仓库所有者可推送。

```powershell
git clone -b develop https://github.com/liqi1222/U585.git
```

## 你需要什么

| 项 | 说明 |
|----|------|
| 硬件 | ST **B-U585I-IOT02A**（板载 ST-LINK/V3E + VCP） |
| 工具链 | CMake 3.22+、Ninja、`arm-none-eabi-gcc`；Windows 上常用 STM32CubeCLT |
| IDE | VS Code：打开 **`U585.code-workspace`**（不要只开 `Secure/` 或 `NonSecure/`） |
| 扩展 | CMake Tools、C/C++、Cortex-Debug |
| 串口 | ST-LINK VCP，本仓库日志按 **COM3、115200 8N1** 采集（你的端口号可能不同） |

CubeMX 工程文件：`U585.ioc`（TrustZone 已开，CubeU5 FW 以 ioc 内版本为准）。

## 目录怎么读

```
Secure/          TrustZone Secure：启动、时钟/电源、GTZC 交接
NonSecure/       NS 应用；文章 demo 在 NonSecure/App/Src/expXX_*.c
Drivers/         HAL/CMSIS（Cube 生成）
tools/           烧录与串口采集脚本
docs/            VS Code 调试、性能基线、实测 log
U585.ioc         CubeMX 唯一配置源
```

- **03–30、33**：演示代码在 NonSecure。
- **31、32**：TrustZone / TF-M，涉及 Secure 侧。
- 双镜像：Secure `0x0C000000`，NonSecure Bank2 `0x08100000`（`TZEN=1`）。

## 构建

在**仓库根目录**（工作区根）执行：

```powershell
cmake --preset Debug
cmake --build --preset Debug
```

产物：

- `Secure/build/U585_S.elf`
- `NonSecure/build/U585_NS.elf`

默认 demo 是 **3**（LED / 按键）。换成第 N 篇：

```powershell
cmake --preset Debug -DU585_ACTIVE_DEMO=5
cmake --build --preset Debug
```

`Performance` 预设用于 05–15 性能基线，见 [`docs/PERFORMANCE_BASELINE_05_15.md`](docs/PERFORMANCE_BASELINE_05_15.md)。

VS Code 打开方式、ST-LINK 路径、F5 调试：[`docs/VS_CODE.md`](docs/VS_CODE.md)。

## 烧录与看 log

板子用 USB 接电脑后：

```powershell
powershell -ExecutionPolicy Bypass -File tools/flash_and_capture.ps1 -Demo 5 -Preset Debug -Seconds 8
```

脚本默认端口 `COM3`。若设备管理器里是别的 COM，加上 `-Port COMx`。

布局自检：

```powershell
python tools/check_experiment_layout.py
```

## 和手册的关系

寄存器与板级引脚不要只看本仓库注释。

- **GitHub 公开仓不含手册**。请自行从 ST 下载 RM0456、UM2839、DS13086、PM0264。
- 作者本机另有 `09-study-and-refs/B-U585I-IOT02A-官方资料/`（不随本仓库公开）。
- 手册典型值 ≠ 本板实测；结论以 `docs/superpowers/measured/demoXX-com3.txt` 为准。

## 下载者请注意

可以 clone、编译、上板对照文章。  
**不能**向本仓库直接推送；没有 Collaborator 写权限。若要改代码，请 fork 到你自己的账号下维护。
