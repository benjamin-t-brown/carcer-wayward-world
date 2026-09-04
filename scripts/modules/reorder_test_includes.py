#!/usr/bin/env python3
"""Put std includes before module imports in __test__ sources."""
from __future__ import annotations

import re
from pathlib import Path

TEST = Path(__file__).resolve().parents[2] / "src" / "__test__"


NEEDED_STD = [
    "#include <functional>",
    "#include <ctime>",
    "#include <cstdlib>",
    "#include <memory>",
    "#include <string_view>",
]


def convert(text: str) -> str:
    text = text.replace("SDL_Color{", "{")
    lines = text.splitlines()
    std: list[str] = []
    setup: list[str] = []
    rest: list[str] = []
    for line in lines:
        m = re.match(r"^\s*#include\s*(<[^>]+>)", line)
        if m:
            inc = "#include " + m.group(1)
            if inc == "#include <SDL.h>":
                continue
            std.append(inc)
            continue
        if "setupTestUi.h" in line:
            setup.append(line.strip())
            continue
        s = line.strip()
        if s in (
            "import carcer;",
            "import sdl2w;",
            "import bmin.string_interop;",
            '#include "macros.h"',
        ):
            continue
        rest.append(line)
    while rest and rest[0].strip() == "":
        rest.pop(0)
    out: list[str] = []
    seen: set[str] = set()
    for s in NEEDED_STD + std:
        if s not in seen:
            seen.add(s)
            out.append(s)
    out.extend(
        [
            "import carcer;",
            "import sdl2w;",
            "import bmin.string_interop;",
            '#include "macros.h"',
        ]
    )
    out.extend(setup)
    out.append("")
    out.extend(rest)
    return "\n".join(out).replace("\r\n", "\n") + "\n"


def main() -> None:
    for p in sorted(TEST.rglob("*.cpp")):
        p.write_text(convert(p.read_text(encoding="utf-8")), encoding="utf-8", newline="\n")
        print(p)


if __name__ == "__main__":
    main()
