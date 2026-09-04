#!/usr/bin/env python3
"""Cast erased LayerManagerInterface::getLayerManager() results."""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
CAST = "static_cast<layers::LayerManager*>"

def fix_text(t: str) -> str:
    # auto layerManager = getLayerManager();
    t = re.sub(
        r"\bauto(\s*\*?\s*)(layerManager)\s*=\s*getLayerManager\(\)",
        rf"auto*\2 = {CAST}(getLayerManager())",
        t,
    )
    t = re.sub(
        r"\bauto(\s*\*?\s*)(layerManager)\s*=\s*state::LayerManagerInterface::getLayerManager\(\)",
        rf"auto*\2 = {CAST}(state::LayerManagerInterface::getLayerManager())",
        t,
    )
    # auto* layerManager = getLayerManager();
    t = re.sub(
        r"\bauto\*\s*(layerManager)\s*=\s*getLayerManager\(\)",
        rf"auto* \1 = {CAST}(getLayerManager())",
        t,
    )
    t = re.sub(
        r"\bauto\*\s*(layerManager)\s*=\s*state::LayerManagerInterface::getLayerManager\(\)",
        rf"auto* \1 = {CAST}(state::LayerManagerInterface::getLayerManager())",
        t,
    )
    return t


def main() -> None:
    n = 0
    for p in (ROOT / "src").rglob("*"):
        if p.suffix not in {".cppm", ".cpp", ".hpp", ".h"}:
            continue
        if "LayerManagerInterface" in p.name:
            continue
        t = p.read_text(encoding="utf-8", errors="replace")
        if "getLayerManager()" not in t:
            continue
        nt = fix_text(t)
        if nt != t:
            p.write_text(nt.replace("\r\n", "\n"), encoding="utf-8", newline="\n")
            n += 1
            print(p.relative_to(ROOT))
    print(f"updated={n}")


if __name__ == "__main__":
    main()
