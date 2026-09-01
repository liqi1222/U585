#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Backfill measured results into U585 articles 18-20 正文.md and rebuild docx."""
from __future__ import annotations

import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(r"E:\360yun\Home\qli\公众号")
PENDING = ROOT / "03-稿件" / "待发布"
BUILD = ROOT / "10-工具与脚本" / "build_wechat_docx.py"

SECTIONS: dict[str, str] = {
    "18": """
## 上板实测记录（2026-07-12）

demo=`18`。I2C2 已迁 NonSecure；LPS22HH 7-bit 地址 `0x5D`。

- `i2c_ready=1`；`WHO_AM_I=179`（**0xB3**），`who_ok=1`
- 已周期读出 `raw_press` / `raw_temp`（例：`raw_press≈4076300`，`raw_temp≈3447`；未做 hPa/°C 工程换算）
- 原始日志：工程 `docs/superpowers/measured/demo18-com3.txt`

上下楼相对高度曲线、工程单位换算仍待补。
""",
    "19": """
## 上板实测记录（2026-07-12）

demo=`19`。ISM330DHCX 7-bit 地址 `0x6B`。

- `i2c_ready=1`；`WHO_AM_I=107`（**0x6B**），`who_ok=1`
- 已周期读出 `ax/ay/az/gx` 原始码（静止时 `az` 约 1.6e4，接近 ±2g 量程下 1g；`ax/ay` 为小负值的 uint32 打印）
- 原始日志：工程 `docs/superpowers/measured/demo19-com3.txt`

量程灵敏度换算、FIFO/DMA、姿态融合仍待补。
""",
    "20": """
## 上板实测记录（2026-07-12）

demo=`20`。IIS2MDC 7-bit 地址 `0x1E`。

- `i2c_ready=1`；`WHO_AM_I=64`（**0x40**），`who_ok=1`
- 已周期读出 `mx/my/mz`（例：约 `455/600/255`）
- 原始日志：工程 `docs/superpowers/measured/demo20-com3.txt`

硬磁/软磁校准、倾斜补偿航向角仍待补。
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
    ok = 0
    for num, section in SECTIONS.items():
        folder = find_article(num)
        if folder is None:
            print(f"MISSING article {num}")
            continue
        md_path = folder / "正文.md"
        text = md_path.read_text(encoding="utf-8-sig")
        text = upsert_section(text, section)
        md_path.write_text(text, encoding="utf-8")
        print(f"updated 正文: {folder.name}")

        r = subprocess.run(
            [sys.executable, str(BUILD), str(folder)],
            cwd=str(ROOT),
            capture_output=True,
        )
        out = (r.stdout or b"") + (r.stderr or b"")
        print(out.decode("utf-8", errors="replace").strip())
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
