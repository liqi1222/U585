#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Backfill one U585 article measured section and rebuild docx."""
from __future__ import annotations

import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(r"E:\360yun\Home\qli\公众号")
PENDING = ROOT / "03-稿件" / "待发布"
BUILD = ROOT / "10-工具与脚本" / "build_wechat_docx.py"

SECTIONS: dict[str, str] = {
    "13": """
## 上板实测记录（2026-07-12）

demo=`13`。SPI2 已迁 NonSecure（原 Cube 4-bit 配置改为 **Mode0 / 8-bit / soft-NSS**）。

| 项 | 结果 |
|----|------|
| 引脚 | PD1=SCK、PD4=MOSI、PD3=MISO、PB12=NSS（WRLS/EMW3080 通路） |
| `spi_ready` | **1** |
| `xfer_ok` | **1**（全双工 4 字节） |
| 探测帧 | `tx0=0x9F`；`rx=[11,0,0,255]`（模组未上电完整协议时 MISO 非 Flash JEDEC，属预期） |

原始日志：工程 `docs/superpowers/measured/demo13-com3.txt`。示波器 CPOL/CPHA 波形、WiFi 正式协议仍待补（见 27 篇）。
""",
    "14": """
## 上板实测记录（2026-07-12）

demo=`14`。ADC1（`GTZC_PERIPH_ADC12`）已迁 NonSecure；本机先做 **内部通道轮询**（`dma_used=0`）。

| 项 | 结果 |
|----|------|
| `adc_ready` | **1** |
| 分辨率 | **12 bit** |
| `vrefint_raw` | ≈**1482**（稳定） |
| `vdda_mv` | ≈**3348**（与 ST-LINK 读电压 3.28 V 接近；工厂 `VREFINT_CAL` 在 NS 侧读数异常，已用 1.212 V 近似换算） |
| `tempsensor_raw` | ≈**930** |

原始日志：工程 `docs/superpowers/measured/demo14-com3.txt`。外部模拟输入 + GPDMA 连续采样仍待补（DMA 搬运见 16 篇）。
""",
    "15": """
## 上板实测记录（2026-07-12）

demo=`15`。DAC1 已迁 NonSecure（`GTZC_PERIPH_DAC1` + PA4 SECCFGR）；本机先做 **直流码输出**（`dma_used=0`），按键可在 0 / 2048 / 4095 间切换。

| 项 | 结果 |
|----|------|
| `dac_ready` | **1** |
| 引脚 | **PA4** = DAC1_OUT1（与 STMod+ SPI1_NSS 共用） |
| `code` / `dor` | **2048 / 2048**（DOR 回读一致） |
| 期望电压 | ≈ VDDA/2（≈1.64 V @ 3.28 V） |
| 万用表/示波器 | **待作者补测**（PA4 引出） |
| TIM+DMA 波形 | **未做**（后续可接 GPDMA） |

原始日志：工程 `docs/superpowers/measured/demo15-com3.txt`。
""",
    "16": """
## 上板实测记录（2026-07-12）

demo=`16`。`GPDMA1` + Channel0 已迁 NonSecure；通道属性为 **NSEC + PRIV**（SRAM3 MPCBB 默认仅特权可访问）。

| 项 | 结果 |
|----|------|
| `dma_ready` | **1** |
| 模式 | 内存到内存，16×word |
| `xfer_ok` | **1** |
| `mismatch` | **0**（`src0==dst0`） |

原始日志：工程 `docs/superpowers/measured/demo16-com3.txt`。外设触发 / linked-list 仍待补。
""",
    "21": """
## 上板实测记录（2026-07-12）

demo=`21`。I2C2 已在 NS；本机补上 **PH1=LPn/xshut** 移交与上电。

| 项 | 结果 |
|----|------|
| `lpn_high` | **1**（PH1 拉高） |
| `probe_ok` | **1**（7-bit 地址 **0x29**，对应 8-bit 0x52；demo12 扫描时未上电故看不到） |
| `is_alive` | **1**（读到 `0xF0` 设备签名；经典 ULD 对 `(0xF0,0x02)` 组合仍需核对） |
| `uld_ready` | **0** |

原始日志：工程 `docs/superpowers/measured/demo21-com3.txt`。**多区测距需加载 ~84 KB ULD 固件**，仍待补。
""",
    "22": """
## 上板实测记录（2026-07-12）

demo=`22`。ADF1 已迁 NonSecure；板载 **MIC1** 走 PE9=CCK0 / PE10=SDI0。本机先做 **轮询采 1 个 PCM 样点**（`dma_used=0`），时钟按 ST BSP：PLL3Q≈11.4 MHz、CCK 分频 4→≈2.86 MHz。

| 项 | 结果 |
|----|------|
| `adf_ready` | **1** |
| `sample_ok` | **1**（`HAL_MDF_PollForAcq`） |
| `sample` | 有符号 PCM 样点（VCP 按 u32 打印，会看到大数/变化值） |
| 滤波 | SINC4，decimation=24 |
| GPDMA 录音 / 双麦 | **未做** |

原始日志：工程 `docs/superpowers/measured/demo22-com3.txt`。
""",
    "23": """
## 上板实测记录（2026-07-12）

demo=`23`。OCTOSPI2 + OCTOSPIM/MEM 已迁 NonSecure；板载 **MX25LM51245G**（Port2）。本机先做 SPI **1-1-1 READ ID (0x9F)**，XIP/八线仍待补。

| 项 | 结果 |
|----|------|
| `ospi_flash_ready` | **1** |
| `jedec_ok` | **1** |
| JEDEC ID | **0xC2 / 0x85 / 0x3A**（Macronix MX25LM51245G） |

原始日志：工程 `docs/superpowers/measured/demo23-com3.txt`。
""",
    "24": """
## 上板实测记录（2026-07-12）

demo=`24`。OCTOSPI1 + OCTOSPIM/MEM 已迁 NonSecure；板载 **APS6408**（Port1 @ `0x90000000`）。

| 项 | 结果 |
|----|------|
| `ospi_psram_ready` | **1**（外设/OSPIM 初始化成功） |
| `memtest_ok` | **0** |
| SPI READ ID / 读写 | 回读为 0（默认八线/同步模式序列仍待按 ST BSP 补齐） |

原始日志：工程 `docs/superpowers/measured/demo24-com3.txt`。完整 octal + 内存映射 + Cache 对比仍待补。
""",
    "25": """
## 上板实测记录（2026-07-12）

demo=`25`。I2C2 已在 NS。**本板 BSP/原理图实际是 M24256（256 Kbit I2C EEPROM @ 7-bit `0x56` / 8-bit `0xAC`），没有 ST25DV NFC 标签。**

| 项 | 结果 |
|----|------|
| `i2c_ready` | **1** |
| ST25DV user `0x53` | **未应答**（`st25_user_ok=0`） |
| ST25DV system `0x57` | **未应答**（`st25_sys_ok=0`） |
| M24256 `0x56` | **`eeprom_ok=1`** |
| 读写校验 | **`rw_ok=1`**（写一字节后读回一致，并恢复原值） |

原始日志：工程 `docs/superpowers/measured/demo25-com3.txt`。NFC/NDEF/手机碰一碰需外接 ST25DV 模块，本板无法测。
""",
    "26": """
## 上板实测记录（2026-07-12）

demo=`26`。UCPD1 已迁 NonSecure（`GTZC_PERIPH_UCPD1` + PA15/PB15 SECCFGR）。本机先做 **Type-C Sink CC 电压态轮询**；USB FS 设备栈（CDC 枚举）仍待补。

| 项 | 结果 |
|----|------|
| `ucpd_ready` | **1** |
| 引脚 | **PA15=CC1**、**PB15=CC2**（模拟） |
| 角色 | Sink（SNK），CC1+CC2 使能 |
| `cc1` / `cc2` | **0 / 0**（本次仅 ST-LINK 供电调试，用户 USB-C 未接 Source） |
| `attached` | **0** |
| USB FS / CDC | **未做** |

原始日志：工程 `docs/superpowers/measured/demo26-com3.txt`。插上 Type-C 充电器或 Host 后应看到非零 VSTATE / `attached=1`；CDC 虚拟串口仍待接 USB Device 中间件。
""",
    "27": """
## 上板实测记录（2026-07-12）

demo=`27`。SPI2 已在 NS；本机补上 **EMW3080 控制脚**（UM2839 Table 14）移交与复位上电。未移植完整 `mx_wifi` / 连 AP。

| 项 | 结果 |
|----|------|
| 引脚 | **PF15=Chip_En**、**PG15=FLOW**、**PD14=NOTIFY**；SPI2=PD1/PD3/PD4/**PB12** |
| `emw_ready` | **1**（Chip_En 拉低 50 ms 后拉高，延时 1.2 s） |
| `spi_ready` | **1** |
| `chip_en` | **1** |
| `flow` / `flow_ok` | **0 / 0**（模组未报 FLOW 就绪） |
| `notify` | **0** |
| `xfer_ok` | **1**（SPI 全双工 4 字节；`rx` 全 0，无 HCI 应答属预期） |
| 连 AP / NetX | **未做** |

原始日志：工程 `docs/superpowers/measured/demo27-com3.txt`。`flow=0` 常见于 **EMW3080 模组固件未刷或版本不匹配**（需 X-WIFI-EMW3080B + SW2 BOOT 更新）；完整 `mx_wifi` 握手与扫 AP 仍待补。
""",
    "28": """
## 上板实测记录（2026-07-12）

demo=`28`。本机先做 **离线 MQTT CONNECT 帧组装**（无 Broker）。完整上云依赖第 27 篇 Wi-Fi（`flow_ok=0`）。**FreeRTOS/RTOS 已移出本系列计划**（纯软件，不涉及板载芯片能力）。

| 项 | 结果 |
|----|------|
| `wifi_ready` | **0** |
| `mqtt_ready` | **0**（未连 Broker） |
| `connect_pkt_len` | **14** |
| `connect_pkt_ok` | **1**（MQTT 3.1.1 CONNECT：`0x10` / remaining `0x0C`） |

原始日志：工程 `docs/superpowers/measured/demo28-com3.txt`。连 Broker / 订阅发布仍待 Wi-Fi。
""",
    "29": """
## 上板实测记录（2026-07-12）

demo=`29`。`GTZC_PERIPH_ICACHE_REG` 已迁 NonSecure。内部 Flash 上做 **关/开 ICACHE** 同一段取指负载，用 DWT `CYCCNT` 计时。

| 项 | 结果 |
|----|------|
| `icache_ready` | **1**（先 Disable 再配 1-way） |
| `cycles_off` | **1481987** |
| `cycles_on` | **820638** |
| `faster` | **1**（约 1.8×） |
| `hit_on` / `miss_on` | **460222 / 70** |
| `hit_off` / `miss_off` | **0 / 0**（Cache 关闭时监视器无命中统计） |

原始日志：工程 `docs/superpowers/measured/demo29-com3.txt`。OSPI XIP 场景下的加速对比仍待补（依赖第 23 篇八线 XIP）。
""",
    "30": """
## 上板实测记录（2026-07-12）

demo=`30`。RNG / AES / PKA 已迁 NonSecure（HSI48→RNG）。本机做 **RNG 抽数 + AES-128-ECB 加解密回环 + PKA 模加**。

| 项 | 结果 |
|----|------|
| `rng_ok` | **1**（4 个随机字互不相同） |
| `aes_ok` | **1**（NIST 样例明文加解密回环一致） |
| `aes_ct0` | 密文前 4 字节拼成的 u32（字节序随 `CRYP_NO_SWAP`） |
| `pka_ok` | **1** |
| `pka_sum0` | **1**（`(5+7) mod 11`；模数为大端字节） |
| HASH / SAES / TLS | **未做** |

原始日志：工程 `docs/superpowers/measured/demo30-com3.txt`。
""",
    "31": """
## 上板实测记录（2026-07-12）

demo=`31`。本工程已是 **CubeMX TrustZone 双镜像**（Secure @ `0x0C000000` → 跳转 NonSecure @ `0x08100000`）。本机从 NS 侧经 **NSC veneer** 回读 SAU/Flash 地图；未改 SAU/option bytes。

| 项 | 结果 |
|----|------|
| `tz_ready` | **1** |
| `nsc_ok` | **1**（`SECURE_GetTzMagic()==0xA5850032`） |
| `sau_regions` | **8** |
| `flash_s` | **0x0C000000** |
| `flash_ns` / `vtor_ns` | **0x08100000** |
| `running_ns` | **1** |
| `gtzc_handoff` | **1**（系列 demo 已用 TZSC 向外设放权） |

原始日志：工程 `docs/superpowers/measured/demo31-com3.txt`。完整 MPCBB 细粒度实验、SecureFault 注入仍可另开专题。
""",
    "32": """
## 上板实测记录（2026-07-12）

demo=`32`。**只做只读盘点，不写 option bytes、不烧 TF-M/SBSFU。** 本仓库是 CubeMX 双镜像 TrustZone，不是 TF-M 参考实现。

| 项 | 结果 |
|----|------|
| `cubemx_tz` | **1** |
| `tfm_ready` | **0**（树内无 TF-M BL2/SPE） |
| `sbsfu_ready` | **0** |
| `secure_boot_ob` | **0**（未改安全启动选项字节） |
| `nsc_ok` | **1**（沿用第 31 篇 NSC） |
| `series_end` | **1** |

原始日志：工程 `docs/superpowers/measured/demo32-com3.txt`。若要真做 TF-M/SBSFU，需另开官方 TF-M 工程并谨慎操作 RDP/OB——本系列刻意止于盘点。
""",
}


def find_article(num: str) -> Path | None:
    for d in PENDING.iterdir():
        if d.is_dir() and re.search(rf"实测笔记-{int(num):02d}-", d.name):
            return d
    return None


def upsert_section(text: str, section: str) -> str:
    marker = "## 上板实测记录（2026-07-12）"
    if marker in text:
        pattern = re.compile(
            r"## 上板实测记录（2026-07-12）\n.*?(?=\n## |\n---\n\n\*以上|\Z)",
            re.S,
        )
        return pattern.sub(section.strip() + "\n\n", text, count=1)
    if "## 参考资料" in text:
        return text.replace("## 参考资料", section.strip() + "\n\n---\n\n## 参考资料", 1)
    return text.rstrip() + "\n\n" + section.strip() + "\n"


def main() -> int:
    if len(sys.argv) < 2:
        print("usage: sync_wechat_one.py <num>")
        return 2
    num = f"{int(sys.argv[1]):02d}"
    if num not in SECTIONS:
        print(f"no section template for {num}")
        return 1
    folder = find_article(num)
    if folder is None:
        print(f"MISSING article {num}")
        return 1
    md_path = folder / "正文.md"
    text = upsert_section(md_path.read_text(encoding="utf-8-sig"), SECTIONS[num])
    md_path.write_text(text, encoding="utf-8")
    print(f"updated 正文: {folder.name}")
    r = subprocess.run([sys.executable, str(BUILD), str(folder)], cwd=str(ROOT))
    return r.returncode


if __name__ == "__main__":
    raise SystemExit(main())
