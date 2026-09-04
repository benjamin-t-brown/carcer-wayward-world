#!/usr/bin/env python3
"""Slim actions/*.cppm preambles the way PlaySound.cppm was cleaned.

Drops converter GMF (cstddef/cstdint/algorithm-for-move), unused string_interop,
and modules already re-exported through AbstractAction. Keeps the public base
(export import AbstractAction / CombatAction), real std headers, sdl2w+macros
when LOG/TRANSLATE/Window are used, and domain imports that are not in the
AbstractAction re-export chain.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
ACTIONS = ROOT / "src" / "actions"

BASE_ABSTRACT = "carcer.state.AbstractAction"
BASE_COMBAT = "carcer.actions.combat.CombatAction"

# export-imported through AbstractAction -> core -> State -> Player/World/MapInstance
REDUNDANT = {
    "bmin.containers",
    "carcer.state.core",
    "carcer.state.State",
    "carcer.db",
    "carcer.model.templates.UtilityTypes",
    "carcer.state.DatabaseInterface",
    "carcer.state.LayerManagerInterface",
    "carcer.model.instances.MapInstance",
    "carcer.model.instances.Player",
    "carcer.model.instances.World",
    "carcer.state.Triggers",
    "carcer.model.Combat",
    "carcer.model.templates.AbilityTypes",
    "carcer.game.map.TileFields",
    "carcer.model.instances.CharacterInstance",
    "carcer.model.instances.ItemInstance",
    "carcer.model.instances.TileInstance",
    "carcer.model.templates.Maps",
    "carcer.model.instances.CharacterPlayer",
    "carcer.model.stats.CharacterStats",
    "carcer.model.templates.CharacterTemplate",
    "carcer.model.templates.Items",
    "carcer.model.templates.RuneTypes",
}

IMPORT_RE = re.compile(r"^(export )?import ([\w.]+);", re.M)
EXPORT_MOD_RE = re.compile(r"^export module ([\w.]+);", re.M)
EXPORT_BRACE_RE = re.compile(r"^export \{", re.M)


def strip_comments(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    text = re.sub(r"//.*?$", "", text, flags=re.M)
    return text


def gmf_headers(scan: str) -> list[str]:
    headers: list[str] = []

    def need(*hs: str) -> None:
        for h in hs:
            if h not in headers:
                headers.append(h)

    if re.search(r"\bstd::(move|forward|swap)\b", scan) or "std::pair" in scan:
        need("utility")
    if "std::optional" in scan:
        need("optional")
    if re.search(
        r"\bstd::(min|max|sort|find|find_if|remove|remove_if|clamp|"
        r"count|count_if|any_of|all_of|none_of)\s*\(",
        scan,
    ):
        need("algorithm")
    if re.search(r"\bstd::make_pair\b", scan):
        need("utility")
    if re.search(r"\bstd::(rand|srand|atoi|atof|malloc|free|exit|abort)\b", scan):
        need("cstdlib")
    if re.search(r"\bstd::(abs|sqrt|floor|ceil|round|pow|sin|cos)\b", scan):
        need("cmath")
    if re.search(r"\bstd::(exception|runtime_error|logic_error|invalid_argument)\b", scan):
        need("stdexcept")
    if re.search(r"\b(typeid|std::type_info|std::type_index)\b", scan):
        need("typeinfo", "typeindex")
    if re.search(r"\bsize_t\b", scan):
        need("cstddef")
    if re.search(r"\b(u?int\d+_t)\b", scan):
        need("cstdint")
    return headers


def cleanup(path: Path) -> bool:
    text = path.read_text(encoding="utf-8").replace("\r\n", "\n")
    em = EXPORT_MOD_RE.search(text)
    brace = EXPORT_BRACE_RE.search(text)
    if not em or not brace:
        print("skip (no export module/brace)", path)
        return False

    mod = em.group(1)
    preamble = text[: brace.start()]
    body = text[brace.start() :].lstrip("\n")
    if not body.endswith("\n"):
        body += "\n"
    scan = strip_comments(body)

    inherits_combat = bool(re.search(r":\s*public\s+CombatAction\b", scan))
    inherits_abstract = bool(re.search(r":\s*public\s+AbstractAction\b", scan))
    need_string_interop = "toStringView" in scan or "fromStringView" in scan
    need_log = bool(re.search(r"\b(LOG|TRANSLATE|THROW_RUNTIME_ERROR)\b", scan))
    need_sdl2w = need_log or "sdl2w::" in scan
    headers = gmf_headers(scan)

    existing: list[tuple[bool, str]] = [
        (bool(m.group(1)), m.group(2)) for m in IMPORT_RE.finditer(preamble)
    ]

    kept_export: list[str] = []
    kept_import: list[str] = []

    def add_export(name: str) -> None:
        if name not in kept_export:
            kept_export.append(name)

    def add_import(name: str) -> None:
        if name not in kept_export and name not in kept_import:
            kept_import.append(name)

    if inherits_combat:
        add_export(BASE_COMBAT)
    if inherits_abstract:
        add_export(BASE_ABSTRACT)

    for exported, name in existing:
        if name in ("sdl2w", "bmin.string_interop", "bmin.containers"):
            continue
        if name in REDUNDANT:
            continue
        if name in (BASE_ABSTRACT, BASE_COMBAT):
            continue
        if exported:
            add_export(name)
        else:
            add_import(name)

    lines: list[str] = []
    if headers:
        lines.append("module;")
        for h in headers:
            lines.append(f"#include <{h}>")
        lines.append("")
    lines.append(f"export module {mod};")
    for name in kept_export:
        lines.append(f"export import {name};")
    if need_sdl2w:
        lines.append("import sdl2w;")
    if need_string_interop:
        lines.append("import bmin.string_interop;")
    for name in kept_import:
        lines.append(f"import {name};")
    if need_log:
        lines.append('#include "macros.h"')
    lines.append("")

    new_text = "\n".join(lines) + "\n" + body
    if new_text == text:
        return False
    path.write_text(new_text, encoding="utf-8", newline="\n")
    return True


def main() -> int:
    files = sorted(ACTIONS.rglob("*.cppm"))
    n = 0
    for p in files:
        if cleanup(p):
            n += 1
            print("cleaned", p.relative_to(ROOT).as_posix())
    print(f"updated {n}/{len(files)} files")
    return 0


if __name__ == "__main__":
    sys.exit(main())
