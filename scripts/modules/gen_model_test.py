#!/usr/bin/env python3
from pathlib import Path
import re

SRC = Path(__file__).resolve().parents[2] / "src"
headers = sorted(SRC.joinpath("model").rglob("*.h"))
stds = set()
bodies = []
for h in headers:
    t = h.read_text(encoding="utf-8")
    for m in re.finditer(r"#include\s*<([^>]+)>", t):
        stds.add(m.group(1))
    t = re.sub(r"#pragma once\n?", "", t)
    t = re.sub(r"#include\s*[<\"][^>\"]+[>\"]\s*\n", "", t)
    t = re.sub(r"namespace sdl2w \{\s*\n\s*class Window;\s*\n\}", "", t)
    t = t.replace("  std::optional<sdl2w::Animation> animation;\n", "")
    bodies.append(f"// {h.relative_to(SRC)}\n" + t)

lines = ["module;"] + [f"#include <{s}>" for s in sorted(stds)]
lines += [
    "#include <optional>",
    "#include <cmath>",
    "#include <cstdint>",
    "#include <cstdlib>",
    "#include <functional>",
    "#include <string_view>",
    "#include <utility>",
    "#include <variant>",
    "#include <array>",
    "#include <algorithm>",
    "",
    "export module carcer.model_test;",
    "export import bmin.containers;",
    "import bmin.string_interop;",
    "",
    "export {",
    "",
]
lines += bodies
lines += ["} // export", ""]
p = SRC / "modules" / "carcer.model_test.cppm"
p.write_text("\n".join(lines), encoding="utf-8", newline="\n")
print("wrote", p, "chars", p.stat().st_size, "headers", len(headers))
