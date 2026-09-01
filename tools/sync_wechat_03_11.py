#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Backfill measured results into U585 articles 03-11 正文.md and rebuild docx."""
from __future__ import annotations

import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(r"E:\360yun\Home\qli\公众号")
PENDING = ROOT / "03-稿件" / "待发布"
BUILD = ROOT / "10-工具与脚本" / "build_wechat_docx.py"

SECTIONS: dict[str, str] = {
    "03": """
## 上板实测记录（2026-07-12）

工程：`Project/U585`，demo=`U585_ACTIVE_DEMO=3`，板载 ST-LINK VCP **COM3 @ 115200 8N1**。

- LED：红 `PH6`、绿 `PH7`（NonSecure GPIO）；按键：`PC13`，未按时 `initial_button=0`
- 现象：红/绿交替翻转；串口有 `[U585][03] led_state=` / `heartbeat=`
- 下载/调试：本机用 STM32CubeProgrammer + VS Code `cortex-debug`（ST-LINK），非 OpenOCD
- 原始日志：工程 `docs/superpowers/measured/demo03-com3.txt`

截图类（CubeMX / VS Code 调试窗口 / 实拍）仍待补。
""",
    "04": """
## 上板实测记录（2026-07-12）

demo=`4`。

- VCP：`[U585][04] SWD/SWO/Fault demo start`，心跳正常
- `itm_trcena=0`（未开调试 TRCENA）；`fault_trigger_enabled=0`（故意 Fault 默认关闭）
- 原始日志：`demo04-com3.txt`

SWO 上位机窗口、Fault 现场截图仍待补。
""",
    "05": """
## 上板实测记录（2026-07-12）

demo=`5`。Secure 侧 MSI→PLL，SCALE1。

| 项 | 读数 |
|----|------|
| SystemCoreClock / SYSCLK / HCLK / PCLK1 / PCLK2 / PCLK3 | **160000000 Hz** |
| FLASH_LATENCY | **4** |

串口波特率正常收发，间接证明 PCLK 可用。MCO 示波器测频仍待补。原始日志：`demo05-com3.txt`。
""",
    "06": """
## 上板实测记录（2026-07-12）

demo=`6`。

| 项 | 读数 |
|----|------|
| smps_selected | **1**（SMPS） |
| voltage_range | **196608**（VOS1 / SCALE1） |
| PWR_CR3 | **2** |
| PWR_VOSR | **507904** |

板级电流（测量跳线 + 万用表）仍待补。原始日志：`demo06-com3.txt`。
""",
    "07": """
## 上板实测记录（2026-07-12）

demo=`7`。

- 已实现：按键边沿 → 短 **Sleep(WFI)** 钩子；`sleep_enabled=1`
- Stop / Standby 进入与 **Idd 电流表读数**仍待补
- 原始日志：`demo07-com3.txt`
""",
    "08": """
## 上板实测记录（2026-07-12）

demo=`8`。RTC 已迁 NonSecure（LSI 日历）。

- `rtc_ready=1`；`backup0=0xA5850008`；`seconds` 递增
- Stop/Standby 唤醒电流仍待补
- 原始日志：`demo08-com3.txt`
""",
    "09": """
## 上板实测记录（2026-07-12）

demo=`9`。

- PC13 **EXTI rising/falling** 已使能；`exti_enabled=1`；`EXTI13_IRQn` 路由 NonSecure
- 按键 press/release 计数需作者按键确认后补截图
- 原始日志：`demo09-com3.txt`
""",
    "10": """
## 上板实测记录（2026-07-12）

demo=`10`。TIM2 已迁 NonSecure。

- `tim_ready=1`；软件 PWM 默认 duty=**20%**（按键切换 20/40/60/80）
- `tim_irq` 约 1 kHz 递增（例：819→3840）
- 示波器波形 / 硬件输入捕获仍待补
- 原始日志：`demo10-com3.txt`
""",
    "11": """
## 上板实测记录（2026-07-12）

demo=`11`。USART1 已迁 NonSecure 直驱 VCP。

- `uart_ready=1`；115200 8N1
- 样例：`[U585][11] printf heartbeat=1 tick=11` …
- 串口助手截图仍待补
- 原始日志：`demo11-com3.txt`
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
        # replace existing block until next ## or end notes
        pattern = re.compile(
            r"## 上板实测记录（2026-07-12）\n.*?(?=\n## |\n---\n\n\*以上|\Z)",
            re.S,
        )
        return pattern.sub(section.strip() + "\n\n", text, count=1)

    # insert before 参考资料
    if "## 参考资料" in text:
        return text.replace("## 参考资料", section.strip() + "\n\n---\n\n## 参考资料", 1)
    return text.rstrip() + "\n\n" + section.strip() + "\n"


def patch_inline_hints(num: str, text: str) -> str:
    if num == "03":
        text = text.replace(
            "（⚠ 待实测：具体引脚、LED 颜色以原理图/实物为准）",
            "（已实测：红 LED=`PH6`、绿 LED=`PH7`）",
        )
        text = text.replace(
            "（⚠ 待实测：按键引脚、按下电平、是否需要上下拉）",
            "（已实测：按键=`PC13`，未按时读数为 0；消抖仍见 09 篇）",
        )
        text = text.replace(
            "> ⚠ 待实测：OpenOCD 的目标配置文件（target cfg）、实际下载命令、cortex-debug 的 `launch.json` 参数，需结合本机环境确认后补全。",
            "> 已实测：本机使用 **ST-LINK_gdbserver + cortex-debug** 与 STM32CubeProgrammer 下载；OpenOCD 非本工程默认路径。",
        )
        text = text.replace(
            "⚠ 待实测：实际 LED 现象（哪颗灯、什么颜色、闪烁是否符合预期）、按键触发是否灵敏、是否需要消抖，上板后记录。",
            "已实测：红/绿 LED 交替闪烁；按键边沿可改闪烁周期（见工程 exp03）；消抖留给 09 篇。",
        )
    if num == "05":
        text = text.replace(
            "⚠ 待实测：具体频率对应的电压档与 latency 取值以 DS13737/RM0456 为准，上板后用稳定运行 + 串口波特率误差佐证。",
            "已实测：SCALE1 + **160 MHz** + Flash latency **4**；VCP 115200 收发正常（见上板实测记录）。",
        )
        text = text.replace(
            "4. 配完要验证（MCO/波特率/定时器），进低功耗实验前固定一套可解释的时钟基线（⚠ 待实测）。",
            "4. 配完要验证（MCO/波特率/定时器）；本机已用 HAL 读数 + 串口验证 160 MHz 基线（MCO 示波器仍可选补）。",
        )
    if num == "06":
        text = text.replace(
            "板上是否有 HSE / LSE 晶振",
            "板上是否有 HSE / LSE 晶振",
        )
    return text


def main() -> int:
    ok = 0
    for num, section in SECTIONS.items():
        folder = find_article(num)
        if folder is None:
            print(f"MISSING article {num}")
            continue
        md_path = folder / "正文.md"
        text = md_path.read_text(encoding="utf-8-sig")
        text = patch_inline_hints(num, text)
        text = upsert_section(text, section)
        md_path.write_text(text, encoding="utf-8")
        print(f"updated 正文: {folder.name}")

        r = subprocess.run(
            [sys.executable, str(BUILD), str(folder)],
            cwd=str(ROOT),
            capture_output=True,
        )
        out = (r.stdout or b"") + (r.stderr or b"")
        try:
            print(out.decode("utf-8", errors="replace").strip())
        except Exception:
            print(f"docx rc={r.returncode}")
        if r.returncode != 0:
            print(f"docx FAILED for {num}: {r.returncode}")
        else:
            ok += 1
            docx = folder / "待发-公众号排版版.docx"
            print(f"docx size={docx.stat().st_size if docx.exists() else 0}")
    print(f"done, docx ok={ok}/{len(SECTIONS)}")
    return 0 if ok == len(SECTIONS) else 1


if __name__ == "__main__":
    raise SystemExit(main())
