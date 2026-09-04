#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
fixed = 0
for p in (ROOT / "src/modules").rglob("*.cppm"):
    t = p.read_text(encoding="utf-8")
    if ("LOG(" in t or "LOG_ENDL" in t or "TRANSLATE" in t) and "macros.h" not in t:
        lines = t.splitlines(True)
        out = []
        done = False
        for line in lines:
            out.append(line)
            if not done and line.startswith("export module "):
                out.append("import sdl2w;\n")
                out.append('#include "macros.h"\n')
                done = True
        p.write_text("".join(out).replace("\r\n", "\n"), encoding="utf-8", newline="\n")
        fixed += 1
        print(p.relative_to(ROOT))
print(f"fixed={fixed}")
