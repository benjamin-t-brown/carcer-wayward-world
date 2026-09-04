#!/usr/bin/env python3
"""Shorten deep carcer.ui.components.* / similar module names that trip GCC."""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SRC = ROOT / "src"
MODULES = SRC / "modules"

# Map deep module names -> shorter aliases
RENAMES = {
    "carcer.ui.components.lists.ListMagicSpells": "carcer.ui.ListMagicSpells",
    "carcer.ui.components.lists.ListInventory": "carcer.ui.ListInventory",
    "carcer.ui.components.lists.ListPickUp": "carcer.ui.ListPickUp",
    "carcer.ui.components.lists.ListChCompactInfoHorizontal": "carcer.ui.ListChCompactInfoHorizontal",
    "carcer.ui.components.lists.ListChCompactInfoVertical": "carcer.ui.ListChCompactInfoVertical",
    "carcer.ui.components.borders.BorderDropShadow": "carcer.ui.BorderDropShadow",
    "carcer.ui.components.borders.BorderInGame": "carcer.ui.BorderInGame",
    "carcer.ui.components.borders.BorderInGameNarrow": "carcer.ui.BorderInGameNarrow",
    "carcer.ui.components.borders.BorderInGameWide": "carcer.ui.BorderInGameWide",
    "carcer.ui.components.borders.BorderModalSmall": "carcer.ui.BorderModalSmall",
    "carcer.ui.components.borders.BorderModalStandard": "carcer.ui.BorderModalStandard",
    "carcer.ui.elements.buttons.ButtonClose": "carcer.ui.ButtonClose",
    "carcer.ui.elements.buttons.ButtonGroup": "carcer.ui.ButtonGroup",
    "carcer.ui.elements.buttons.ButtonIcon": "carcer.ui.ButtonIcon",
    "carcer.ui.elements.buttons.ButtonList": "carcer.ui.ButtonList",
    "carcer.ui.elements.buttons.ButtonModal": "carcer.ui.ButtonModal",
    "carcer.ui.elements.buttons.ButtonMove": "carcer.ui.ButtonMove",
    "carcer.ui.elements.buttons.ButtonScroll": "carcer.ui.ButtonScroll",
    "carcer.ui.elements.buttons.ButtonSprite": "carcer.ui.ButtonSprite",
    "carcer.ui.elements.buttons.ButtonTextWrap": "carcer.ui.ButtonTextWrap",
    "carcer.ui.elements.buttons.ButtonWorldAction": "carcer.ui.ButtonWorldAction",
}


def main() -> None:
    # Rename in all text sources under src/
    files = list(SRC.rglob("*"))
    for p in files:
        if not p.is_file():
            continue
        if p.suffix not in {".cppm", ".cpp", ".mk", ".list", ".txt", ".md", ".h", ".hpp"}:
            continue
        t = p.read_text(encoding="utf-8", errors="replace")
        nt = t
        for old, new in RENAMES.items():
            nt = nt.replace(old, new)
        if nt != t:
            p.write_text(nt.replace("\r\n", "\n"), encoding="utf-8", newline="\n")
            print("updated", p.relative_to(ROOT))

    # Files are colocated (ButtonScroll.cppm next to ButtonScroll.cpp); module
    # *names* are rewritten in-place above. Do not rename files to dotted module names.

    print("done")


if __name__ == "__main__":
    main()
