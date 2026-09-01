#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Restore U585 articles 28-32 from git HEAD originals (29-33) with single renumber."""
from __future__ import annotations

import re
import subprocess
from pathlib import Path

WECHAT = Path(r"E:\360yun\Home\qli\公众号")
PENDING = WECHAT / "03-稿件" / "待发布"
FULL = {29: 28, 30: 29, 31: 30, 32: 31, 33: 32}


def git_files(pattern: str = "03-稿件/待发布") -> list[str]:
    out = subprocess.check_output(
        ["git", "-c", "core.quotepath=false", "ls-files", "-z", "--", pattern],
        cwd=WECHAT,
    )
    return [p.replace("\\", "/") for p in out.decode("utf-8").split("\0") if p]


def find_old_dirs() -> dict[int, str]:
    """Map old article number -> git directory path (posix)."""
    found: dict[int, str] = {}
    for p in git_files():
        m = re.search(r"(03-稿件/待发布/[^/]*实测笔记-(\d{2})-[^/]+)/", p)
        if not m:
            continue
        num = int(m.group(2))
        if num in FULL:  # originals 29-33
            found[num] = m.group(1)
    return found


def find_folder(num: int) -> Path:
    for d in PENDING.iterdir():
        if d.is_dir() and re.search(rf"实测笔记-{num:02d}-", d.name):
            return d
    raise FileNotFoundError(num)


def apply_full(text: str) -> str:
    for old, new in FULL.items():
        tok = f"__N{new}__"
        o = f"{old:02d}"
        for a, b in [
            (f"实测笔记（{old}）", f"实测笔记（{tok}）"),
            (f"实测笔记-{o}-", f"实测笔记-{tok}-"),
            (f"series_index: {old}", f"series_index: {tok}"),
            (f"第 {old} 篇", f"第 {tok} 篇"),
            (f"第{old}篇", f"第{tok}篇"),
            (f"demo=`{o}`", f"demo=`{tok}`"),
            (f"demo{o}-com3", f"demo{tok}-com3"),
            (f"[U585][{o}]", f"[U585][{tok}]"),
            (f"（{old}）：", f"（{tok}）："),
            (f"笔记（{old}）", f"笔记（{tok}）"),
        ]:
            text = text.replace(a, b)
    for new in FULL.values():
        text = text.replace(f"__N{new}__", str(new))
    for new in FULL.values():
        n = f"{new:02d}"
        text = text.replace(f"实测笔记-{new}-", f"实测笔记-{n}-")
        text = text.replace(f"demo=`{new}`", f"demo=`{n}`")
        text = text.replace(f"demo{new}-com3", f"demo{n}-com3")
        text = text.replace(f"[U585][{new}]", f"[U585][{n}]")
    text = text.replace("33 篇", "32 篇")
    text = text.replace("全 33", "全 32")
    text = text.replace("31–33", "30–32")
    text = text.replace("31-33", "30-32")
    text = text.replace("32–33", "31–32")
    text = text.replace("（33）", "（32）")
    # drop RTOS from series recap wording if present
    text = text.replace("WiFi/RTOS/MQTT", "WiFi/MQTT")
    text = text.replace("存储连接(23-29)", "存储连接(23-28)")
    text = text.replace("性能与安全(30-33)", "性能与安全(29-32)")
    text = text.replace("性能与安全(30-32)", "性能与安全(29-32)")
    return text


def restore_one(old: int, new: int, old_dirs: dict[int, str]) -> None:
    old_dir = old_dirs[old]
    cur = find_folder(new)
    print(f"{old} -> {new}: {Path(old_dir).name} => {cur.name}")
    prefix = old_dir.rstrip("/") + "/"
    files = [p for p in git_files() if p.startswith(prefix)]
    for path in files:
        rel = path[len(prefix) :]
        blob = subprocess.check_output(["git", "show", f"HEAD:{path}"], cwd=WECHAT)
        dest = cur / rel
        dest.parent.mkdir(parents=True, exist_ok=True)
        if rel.lower().endswith((".docx", ".png", ".jpg", ".jpeg", ".gif", ".webp", ".pdf")):
            dest.write_bytes(blob)
            continue
        try:
            text = blob.decode("utf-8")
        except UnicodeDecodeError:
            dest.write_bytes(blob)
            continue
        dest.write_text(apply_full(text), encoding="utf-8")
    print(f"  wrote {len(files)} files")


def main() -> None:
    old_dirs = find_old_dirs()
    print("old dirs:", {k: Path(v).name for k, v in sorted(old_dirs.items())})
    for old, new in FULL.items():
        if old not in old_dirs:
            raise SystemExit(f"missing git dir for {old}")
        restore_one(old, new, old_dirs)

    # fix series-recap explicitly
    recap = find_folder(32) / "assets" / "series-recap.svg"
    if recap.exists():
        t = apply_full(recap.read_text(encoding="utf-8"))
        t = t.replace("存储连接(23-29)：Flash/PSRAM/NFC/USB/WiFi/MQTT",
                      "存储连接(23-28)：Flash/PSRAM/NFC/USB/WiFi/MQTT")
        t = t.replace("存储连接(23-28)：Flash/PSRAM/NFC/USB/WiFi/RTOS/MQTT",
                      "存储连接(23-28)：Flash/PSRAM/NFC/USB/WiFi/MQTT")
        recap.write_text(t, encoding="utf-8")

    for n in range(28, 33):
        t = (find_folder(n) / "正文.md").read_text(encoding="utf-8")
        title = re.search(r"^title:.*$", t, re.M).group(0)
        idx = re.search(r"^series_index:.*$", t, re.M).group(0)
        print("OK", n, title, idx)


if __name__ == "__main__":
    main()
