#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Renumber U585 series after dropping FreeRTOS: old 29..33 -> new 28..32."""
from __future__ import annotations

import re
import shutil
from pathlib import Path

U585 = Path(r"E:\360yun\Home\qli\Project\U585")
WECHAT = Path(r"E:\360yun\Home\qli\公众号")
PENDING = WECHAT / "03-稿件" / "待发布"
CANCELLED = WECHAT / "03-稿件" / "取消"

# old_num -> new_num (article / demo id)
MAP = {29: 28, 30: 29, 31: 30, 32: 31, 33: 32}

STEMS = {
    29: "mqtt_cloud",
    30: "icache_fetch",
    31: "rng_aes_pka",
    32: "trustzone_gtzc",
    33: "tfm_secure_boot",
}

# Per-demo state magic was 0xA5850000 | old_num; shift with article number.
# TrustZone NSC cookie 0xA5850032 stays unchanged (Secure side constant).
MAGIC = {
    29: (0xA585001D, 0xA585001C),  # MQTT 29->28
    30: (0xA585001E, 0xA585001D),
    31: (0xA585001F, 0xA585001E),
    32: (0xA5850020, 0xA585001F),
    33: (0xA5850021, 0xA5850020),
}


def apply_map_placeholders(text: str) -> str:
    """Replace old->new without cascading (30->29 then 29->28)."""
    for old, new in MAP.items():
        token = f"__RENUM_{new}__"
        o, n = f"{old:02d}", f"{new:02d}"
        # Prefer specific patterns; order within one old doesn't cascade across MAP
        reps = [
            (f"实测笔记（{old}）", f"实测笔记（{token}）"),
            (f"实测笔记-{o}-", f"实测笔记-{token}-"),
            (f"笔记（{old}）", f"笔记（{token}）"),
            (f"第 {old} 篇", f"第 {token} 篇"),
            (f"第{old}篇", f"第{token}篇"),
            (f"series_index: {old}", f"series_index: {token}"),
            (f"demo=`{o}`", f"demo=`{token}`"),
            (f"demo=`{old}`", f"demo=`{token}`"),
            (f"demo{o}-com3", f"demo{token}-com3"),
            (f"[U585][{o}]", f"[U585][{token}]"),
            (f"U585_Demo_Exp{o}", f"U585_Demo_Exp{token}"),
            (f"U585_Exp{o}State", f"U585_Exp{token}State"),
            (f"g_u585_exp{o}_state", f"g_u585_exp{token}_state"),
            (f"exp{o}_", f"exp{token}_"),
            (f"App/Src/exp{o}_", f"App/Src/exp{token}_"),
            (f'"{o}"', f'"{token}"'),  # demo id in U585_Demo
            (f"U585_ACTIVE_DEMO == {old}", f"U585_ACTIVE_DEMO == {token}"),
            (f"    \"{o}\":", f"    \"{token}\":"),
            (f"| {old} | `{old}` |", f"| {token} | `{token}` |"),
            (f"| {old} | `{o}` |", f"| {token} | `{token}` |"),
            (f"{old}: \"exp{o}_", f"{token}: \"exp{token}_"),
            (f"（{old}）：", f"（{token}）："),
            (f"（{old}）", f"（{token}）"),  # titles / links
            (f"篇 {old}", f"篇 {token}"),
            (f"31–33", f"30–32"),  # safety range; applied once via MAP loop ok if only when old==31
        ]
        for a, b in reps:
            text = text.replace(a, b)
        old_m, new_m = MAGIC[old]
        text = text.replace(f"0x{old_m:08X}UL", f"0x{new_m:08X}UL")
        text = text.replace(f"0x{old_m:08x}UL", f"0x{new_m:08x}UL")

    for new in MAP.values():
        text = text.replace(f"__RENUM_{new}__", f"{new:02d}" if False else str(new))
    # Zero-pad tokens that should stay 02d
    for new in MAP.values():
        # After replace, bare new is fine for most; fix paths that need 02d
        pass

    # Fix zero-padding: we intentionally used str(new) for article numbers (28 not 028).
    # But demo ids and file stems need 02d. Re-apply 02d for known patterns where we used token as bare number.
    for new in sorted(MAP.values()):
        n = f"{new:02d}"
        # Patterns that must be zero-padded
        text = text.replace(f"实测笔记-{new}-", f"实测笔记-{n}-")
        text = text.replace(f"demo=`{new}`", f"demo=`{n}`")
        text = text.replace(f"demo{new}-com3", f"demo{n}-com3")
        text = text.replace(f"[U585][{new}]", f"[U585][{n}]")
        text = text.replace(f"U585_Demo_Exp{new}", f"U585_Demo_Exp{n}")
        text = text.replace(f"U585_Exp{new}State", f"U585_Exp{n}State")
        text = text.replace(f"g_u585_exp{new}_state", f"g_u585_exp{n}_state")
        text = text.replace(f"exp{new}_", f"exp{n}_")
        text = text.replace(f'"{new}"', f'"{n}"')  # may over-pad unrelated — only in demo structs usually "28"
        text = text.replace(f'    "{new}":', f'    "{n}":')
        text = text.replace(f"| {new} | `{new}` |", f"| {new} | `{n}` |")
        text = text.replace(f"{new}: \"exp{n}_", f"{new}: \"exp{n}_")
    return text


def apply_map_c_source(text: str, old: int, new: int) -> str:
    """Renumber a single C source that only contains its own demo number."""
    o, n = f"{old:02d}", f"{new:02d}"
    reps = [
        (f"U585_Exp{o}State", f"U585_Exp{n}State"),
        (f"g_u585_exp{o}_state", f"g_u585_exp{n}_state"),
        (f"exp{o}_", f"exp{n}_"),
        (f"U585_Demo_Exp{o}", f"U585_Demo_Exp{n}"),
        (f"[U585][{o}]", f"[U585][{n}]"),
        (f'  "{o}",', f'  "{n}",'),
        (f'"{o}"', f'"{n}"'),
    ]
    for a, b in reps:
        text = text.replace(a, b)
    old_m, new_m = MAGIC[old]
    text = text.replace(f"0x{old_m:08X}UL", f"0x{new_m:08X}UL")
    return text


def firmware_rename_sources() -> None:
    src_dir = U585 / "NonSecure" / "App" / "Src"
    temps: list[tuple[Path, Path, int]] = []
    for old, new in MAP.items():
        stem = STEMS[old]
        src = src_dir / f"exp{old:02d}_{stem}.c"
        if not src.exists():
            raise FileNotFoundError(src)
        tmp = src_dir / f"_renum_exp{new:02d}_{stem}.c"
        tmp.write_text(apply_map_c_source(src.read_text(encoding="utf-8"), old, new), encoding="utf-8")
        temps.append((src, tmp, old))
    for src, tmp, old in temps:
        new = MAP[old]
        src.unlink()
        final = src_dir / f"exp{new:02d}_{STEMS[old]}.c"
        tmp.rename(final)
        print(f"FW: {src.name} -> {final.name}")


def firmware_measured_logs() -> None:
    meas = U585 / "docs" / "superpowers" / "measured"
    for old, new in sorted(MAP.items(), reverse=True):
        src = meas / f"demo{old:02d}-com3.txt"
        if not src.exists():
            print(f"skip missing {src.name}")
            continue
        text = src.read_text(encoding="utf-8")
        text = text.replace(f"[U585][{old:02d}]", f"[U585][{new:02d}]")
        text = re.sub(rf"\bdemo\s+{old}\b", f"demo {new}", text)
        text = text.replace(f"demo{old:02d}", f"demo{new:02d}")
        dst = meas / f"demo{new:02d}-com3.txt"
        dst.write_text(text, encoding="utf-8")
        if src.resolve() != dst.resolve():
            src.unlink()
        print(f"measured: demo{old:02d} -> demo{new:02d}")


def firmware_update_maps() -> None:
    cmake = U585 / "NonSecure" / "CMakeLists.txt"
    t = cmake.read_text(encoding="utf-8")
    t = t.replace(
        "STRINGS 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 29 30 31 32 33",
        "STRINGS 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32",
    )
    for old, new in sorted(MAP.items(), reverse=True):
        stem = STEMS[old]
        t = t.replace(f"App/Src/exp{old:02d}_{stem}.c", f"App/Src/exp{new:02d}_{stem}.c")
    cmake.write_text(t, encoding="utf-8")

    for rel in [
        "NonSecure/App/Inc/u585_demo.h",
        "NonSecure/App/Src/u585_app.c",
        "tools/check_experiment_layout.py",
        "tools/sync_wechat_one.py",
        "U585_EXPERIMENTS.md",
    ]:
        path = U585 / rel
        text = apply_map_placeholders(path.read_text(encoding="utf-8"))
        if rel.endswith("check_experiment_layout.py"):
            text = text.replace('["32", "33", "NonSecure", "TrustZone"]', '["32", "NonSecure", "TrustZone"]')
            text = text.replace('["31", "32", "NonSecure", "TrustZone"]', '["31", "32", "NonSecure", "TrustZone"]')
        if rel.endswith("U585_EXPERIMENTS.md"):
            text = re.sub(
                r"\| 28 \| — \| — \| \*\*Out of scope\*\*[^\n]*\n",
                "",
                text,
            )
            text = text.replace("| 32–33 |", "| 31–32 |")
            text = text.replace("Articles 32 and 33", "Articles 31 and 32")
            text = text.replace("3–27, 29–33", "3–32")
            text = text.replace("3-27, 29-33", "3-32")
        path.write_text(text, encoding="utf-8")
        print(f"updated {rel}")


def wechat_shift_folders() -> None:
    CANCELLED.mkdir(parents=True, exist_ok=True)
    for d in list(PENDING.iterdir()):
        if d.is_dir() and re.search(r"实测笔记-28-FreeRTOS", d.name):
            dest = CANCELLED / d.name
            if dest.exists():
                shutil.rmtree(dest)
            shutil.move(str(d), str(dest))
            # Mark cancelled article — keep historical number 28 but note cancelled
            body = dest / "正文.md"
            if body.exists():
                t = body.read_text(encoding="utf-8")
                t = t.replace("status: ready", "status: cancelled")
                t = t.replace("status: draft", "status: cancelled")
                if "status: cancelled" not in t and t.startswith("---"):
                    t = t.replace("---\n", "---\nstatus: cancelled\n", 1)
                body.write_text(t, encoding="utf-8")
            print(f"cancelled -> {dest.name}")

    def find(num: int) -> Path:
        for d in PENDING.iterdir():
            if d.is_dir() and re.search(rf"实测笔记-{num:02d}-", d.name):
                return d
        raise FileNotFoundError(f"article folder {num}")

    temps: list[tuple[Path, int, int]] = []
    for old, new in MAP.items():
        src = find(old)
        tmp = PENDING / f"_renum_{new:02d}__{src.name}"
        shutil.move(str(src), str(tmp))
        temps.append((tmp, old, new))

    for tmp, old, new in temps:
        m = re.match(r"_renum_\d{2}__(.+)", tmp.name)
        original = m.group(1) if m else tmp.name
        new_name = re.sub(rf"实测笔记-{old:02d}-", f"实测笔记-{new:02d}-", original)
        final = PENDING / new_name
        if final.exists():
            raise FileExistsError(final)
        shutil.move(str(tmp), str(final))
        print(f"folder: ...{old:02d}... -> {new_name}")

        for path in final.rglob("*"):
            if not path.is_file():
                continue
            if path.suffix.lower() in {".docx", ".png", ".jpg", ".jpeg", ".gif", ".webp", ".pdf"}:
                continue
            try:
                text = path.read_text(encoding="utf-8")
            except Exception:
                continue
            # Only remap THIS article's own number first; global pass handles xrefs
            new_text = text
            new_text = new_text.replace(f"series_index: {old}", f"series_index: {new}")
            new_text = new_text.replace(f"实测笔记（{old}）", f"实测笔记（{new}）")
            new_text = new_text.replace(f"实测笔记-{old:02d}-", f"实测笔记-{new:02d}-")
            new_text = new_text.replace(f"第 {old} 篇", f"第 {new} 篇")
            new_text = new_text.replace(f"demo=`{old:02d}`", f"demo=`{new:02d}`")
            new_text = new_text.replace(f"demo{old:02d}-com3", f"demo{new:02d}-com3")
            new_text = new_text.replace(f"[U585][{old:02d}]", f"[U585][{new:02d}]")
            if new_text != text:
                path.write_text(new_text, encoding="utf-8")


def wechat_global_xrefs() -> None:
    """Shift remaining cross-refs; skip cancelled FreeRTOS folder (historical 28)."""
    skip_dirs = {CANCELLED.resolve()}
    roots = [
        WECHAT / "02-选题与排期",
        WECHAT / "03-稿件",
        WECHAT / "README.md",
        WECHAT / "docs",
        WECHAT / "01-素材与资产",
    ]
    files: list[Path] = []
    for root in roots:
        if root.is_file():
            files.append(root)
            continue
        if not root.is_dir():
            continue
        for p in root.rglob("*"):
            if not p.is_file():
                continue
            if any(s in p.resolve().parents or p.resolve().parent == s for s in skip_dirs):
                # Still update series count inside cancelled? skip body renumbers of 29-33
                if CANCELLED.resolve() in p.resolve().parents or p.resolve().parent == CANCELLED.resolve():
                    # Only update "33 篇" counts in cancelled folder
                    try:
                        t = p.read_text(encoding="utf-8")
                    except Exception:
                        continue
                    nt = t.replace("33 篇", "32 篇").replace("全 33", "全 32")
                    if nt != t:
                        p.write_text(nt, encoding="utf-8")
                    continue
            if p.suffix.lower() in {".md", ".txt", ".svg", ".py", ".html"}:
                files.append(p)

    for path in files:
        if CANCELLED.resolve() in path.resolve().parents:
            continue
        try:
            text = path.read_text(encoding="utf-8")
        except Exception:
            continue
        orig = text
        text = apply_map_placeholders(text)
        # Series length
        text = text.replace("33 篇", "32 篇")
        text = text.replace("全 33", "全 32")
        text = text.replace("共 33", "共 32")
        text = text.replace("后续 33 篇", "后续 32 篇")
        text = text.replace("## 33 篇索引", "## 32 篇索引")
        text = text.replace("1-33", "1-32")
        text = text.replace("03–33", "03–32")
        text = text.replace("3-33", "3-32")
        # Range phrases that may remain
        text = text.replace("31–33", "30–32")
        text = text.replace("31-33", "30-32")
        text = text.replace("32–33", "31–32")
        text = text.replace("（33）", "（32）")  # remaining bare
        if text != orig:
            path.write_text(text, encoding="utf-8")
            try:
                rel = path.relative_to(WECHAT)
            except ValueError:
                rel = path
            print(f"xref: {rel}")


def fix_选题池_rows() -> None:
    path = WECHAT / "02-选题与排期" / "选题池.md"
    if not path.exists():
        return
    t = path.read_text(encoding="utf-8")
    # Ensure cancelled FreeRTOS row stays as historical note without claiming slot 28 for MQTT twice
    # MQTT should now be 28, etc. apply_map_placeholders already did （29）->（28）
    # Fix progress blurb if still says 33
    t = t.replace("全 33 篇", "全 32 篇")
    t = t.replace("全33篇", "全32篇")
    path.write_text(t, encoding="utf-8")


def verify() -> None:
    src = U585 / "NonSecure" / "App" / "Src"
    expected = [
        "exp28_mqtt_cloud.c",
        "exp29_icache_fetch.c",
        "exp30_rng_aes_pka.c",
        "exp31_trustzone_gtzc.c",
        "exp32_tfm_secure_boot.c",
    ]
    for name in expected:
        if not (src / name).exists():
            raise SystemExit(f"MISSING {name}")
    for old in MAP:
        stem = STEMS[old]
        if (src / f"exp{old:02d}_{stem}.c").exists() and old not in MAP.values():
            # old 33 file shouldn't exist
            pass
    if (src / "exp33_tfm_secure_boot.c").exists():
        raise SystemExit("old exp33 still present")
    if (src / "exp29_mqtt_cloud.c").exists():
        raise SystemExit("old exp29 mqtt still present")
    print("verify OK")


def main() -> None:
    firmware_rename_sources()
    firmware_measured_logs()
    firmware_update_maps()
    wechat_shift_folders()
    wechat_global_xrefs()
    fix_选题池_rows()
    verify()
    print("DONE")


if __name__ == "__main__":
    main()
