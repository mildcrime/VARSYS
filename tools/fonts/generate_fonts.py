#!/usr/bin/env python3
# ============================================================================
#  generate_fonts.py — Сборка LVGL-шрифтов VARSYS (iOS-стиль)
#
#  Источники:
#    - Inter (OFL)            — латиница + кириллица + пунктуация
#    - Tabler Icons (MIT)     — иконки интерфейса (те же, что в прототипе)
#
#  Запуск:  python3 generate_fonts.py
#  Требует: ./node_modules/.bin/lv_font_conv, ./inter/*.ttf,
#           ./package/dist/fonts/tabler-icons.ttf (+ tabler-icons.css)
#
#  Результат: ../../src/ui/fonts/varsys_NN.c  и  varsys_fonts.h
# ============================================================================
import os, re, subprocess, sys

HERE = os.path.dirname(os.path.abspath(__file__))
OUT  = os.path.normpath(os.path.join(HERE, "..", "..", "src", "ui", "fonts"))
CONV = os.path.join(HERE, "node_modules", ".bin", "lv_font_conv")
CSS  = os.path.join(HERE, "package", "dist", "tabler-icons.css")
TABLER = os.path.join(HERE, "package", "dist", "fonts", "tabler-icons.ttf")
INTER  = lambda w: os.path.join(HERE, "inter", f"Inter-{w}.ttf")

# Размер -> начертание Inter
SIZES = [(12, "Regular"), (14, "Regular"), (16, "Regular"), (22, "SemiBold")]
BPP = 4

# Диапазоны текста: латиница, кириллица, типографская пунктуация
TEXT_RANGES = [
    "0x20-0x7E",            # ASCII
    "0x0400-0x045F",        # кириллица (А-я, ё)
    "0x2116", "0x00B0", "0x00B7", "0x2013", "0x2014", "0x2022", "0x2212", "0x2026",
]

# Макрос -> имя иконки Tabler (с запасными вариантами)
ICONS = {
    "ICON_ANTENNA":   ["antenna"],
    "ICON_BLUETOOTH": ["bluetooth"],
    "ICON_INFRARED":  ["wave-sine"],
    "ICON_NFC":       ["nfc"],
    "ICON_WIFI":      ["wifi"],
    "ICON_FOLDER":    ["folder"],
    "ICON_APPS":      ["apps"],
    "ICON_SETTINGS":  ["settings"],
    "ICON_BACK":      ["chevron-left"],
    "ICON_CHEVRON":   ["chevron-right"],
    "ICON_BATTERY":   ["battery-3"],
    "ICON_SOUND":     ["volume"],
    "ICON_VIBRO":     ["device-watch"],
    "ICON_MOON":      ["moon"],
    "ICON_BRIGHTNESS":["sun"],
    "ICON_INFO":      ["info-circle"],
    "ICON_SCAN":      ["radar-2", "radar"],
    "ICON_RECORD":    ["circle-dot"],
    "ICON_SAVE":      ["device-floppy"],
    "ICON_SIGNAL":    ["antenna-bars-5"],
    "ICON_LANGUAGE":  ["language", "world"],
    "ICON_EXPERT":    ["alert-triangle"],
}

def parse_css():
    txt = open(CSS, encoding="utf-8").read()
    table = dict(re.findall(r'\.ti-([a-z0-9-]+):before\s*\{\s*content:\s*"\\([0-9a-fA-F]+)"', txt))
    return {name: int(code, 16) for name, code in table.items()}

def resolve_icons(table):
    out = {}
    for macro, names in ICONS.items():
        for n in names:
            if n in table:
                out[macro] = (n, table[n]); break
        else:
            sys.exit(f"icon not found for {macro}: tried {names}")
    return out

def utf8_literal(cp):
    return "".join(f"\\x{b:02X}" for b in chr(cp).encode("utf-8"))

def build_font(size, weight, icon_cps):
    out_c = os.path.join(OUT, f"varsys_{size}.c")
    cmd = [CONV, "--font", INTER(weight)]
    for r in TEXT_RANGES:
        cmd += ["-r", r]
    cmd += ["--font", TABLER]
    for cp in icon_cps:
        cmd += ["-r", hex(cp)]
    cmd += ["--size", str(size), "--bpp", str(BPP),
            "--format", "lvgl", "--lv-font-name", f"varsys_{size}", "-o", out_c]
    subprocess.run(cmd, check=True)
    return out_c

def write_header(icons):
    path = os.path.join(OUT, "varsys_fonts.h")
    lines = [
        "// ===========================================================================",
        "//  varsys_fonts.h — Шрифты и иконки VARSYS (СГЕНЕРИРОВАНО generate_fonts.py)",
        "//  НЕ редактировать вручную. Пересборка: tools/fonts/generate_fonts.py",
        "//  Inter (OFL) + Tabler Icons (MIT). UTF-8 строки для меток LVGL.",
        "// ===========================================================================",
        "#pragma once",
        "#include <lvgl.h>",
        "",
    ]
    for size, _ in SIZES:
        lines.append(f"LV_FONT_DECLARE(varsys_{size});")
    lines.append("")
    lines.append("// --- Иконки (Tabler) ---")
    width = max(len(m) for m in icons) + 1
    for macro, (name, cp) in icons.items():
        lines.append(f'#define {macro:<{width}} "{utf8_literal(cp)}"   // ti-{name} U+{cp:04X}')
    lines.append("")
    open(path, "w", encoding="utf-8").write("\n".join(lines))
    return path

def main():
    os.makedirs(OUT, exist_ok=True)
    table = parse_css()
    icons = resolve_icons(table)
    icon_cps = sorted({cp for _, cp in icons.values()})
    for size, weight in SIZES:
        print(f"-> varsys_{size}.c  (Inter-{weight}, {len(icon_cps)} icons)")
        build_font(size, weight, icon_cps)
    hdr = write_header(icons)
    print("-> " + os.path.relpath(hdr, HERE))
    print("done")

if __name__ == "__main__":
    main()
