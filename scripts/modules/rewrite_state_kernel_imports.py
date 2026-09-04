#!/usr/bin/env python3
"""Rewrite folded carcer.state.* kernel imports to carcer.state and dedupe."""

from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SRC = ROOT / "src"

OLD = [
    "carcer.state.StateManagerInterface",
    "carcer.state.LayerManagerInterface",
    "carcer.state.DatabaseInterface",
    "carcer.state.AbstractAction",
    "carcer.state.StateManager",
    "carcer.state.ActionBus",
    "carcer.state.UiManager",
    "carcer.state.Triggers",
    "carcer.state.core",
    "carcer.state.State",
]

IMPORT_RE = re.compile(r"^((?:export\s+)?import\s+)(carcer\.state)\s*;\s*$")

SKIP_PREFIX = ("lib/sdl2w/", "lib/bmin/")


def rewrite_text(text: str) -> str:
    for old in OLD:
        text = text.replace(old, "carcer.state")
    lines = text.splitlines(keepends=True)
    want_export = False
    want_import = False
    for line in lines:
        m = IMPORT_RE.match(line.replace("\r", ""))
        if not m:
            continue
        if line.lstrip().startswith("export "):
            want_export = True
        else:
            want_import = True
    if want_export:
        want_import = False
    emitted = False
    out: list[str] = []
    for line in lines:
        m = IMPORT_RE.match(line.replace("\r", ""))
        if not m:
            out.append(line)
            continue
        if emitted:
            continue
        emitted = True
        nl = "\n" if line.endswith("\n") else ""
        if want_export:
            out.append(f"export import carcer.state;{nl}")
        elif want_import:
            out.append(f"import carcer.state;{nl}")
    return "".join(out)


def main() -> int:
    n = 0
    for p in SRC.rglob("*"):
        if p.suffix not in {".cppm", ".cpp", ".h"}:
            continue
        rel = p.relative_to(SRC).as_posix()
        if rel.startswith(SKIP_PREFIX):
            continue
        text = p.read_text(encoding="utf-8").replace("\r\n", "\n")
        new = rewrite_text(text)
        if new != text:
            if not new.endswith("\n"):
                new += "\n"
            p.write_text(new, encoding="utf-8", newline="\n")
            n += 1
            print("rewrote", rel)
    print(f"updated {n} files")
    return 0


if __name__ == "__main__":
    sys.exit(main())
