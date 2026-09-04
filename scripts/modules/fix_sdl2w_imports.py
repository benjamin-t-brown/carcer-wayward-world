#!/usr/bin/env python3
from pathlib import Path

mods = Path("src/modules")
fixed = 0
for p in mods.glob("carcer*.cppm"):
    t = p.read_text(encoding="utf-8")
    if "sdl2w::" not in t:
        continue
    if "import sdl2w;" in t:
        continue
    lines = t.splitlines(True)
    out = []
    done = False
    for line in lines:
        out.append(line)
        if not done and line.startswith("export module "):
            out.append("export import sdl2w;\n")
            out.append('#include "macros.h"\n')
            done = True
            fixed += 1
    if done:
        p.write_text("".join(out), encoding="utf-8", newline="\n")
print("fixed", fixed)
