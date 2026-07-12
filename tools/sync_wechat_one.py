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
