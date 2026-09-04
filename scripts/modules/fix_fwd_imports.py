#!/usr/bin/env python3
"""Replace cross-module forward decls with export import of the owning module."""
from pathlib import Path
import re

FWD = re.compile(
    r"(?m)^[ \t]*(?:struct|class)[ \t]+([A-Za-z_][A-Za-z0-9_]*)[ \t]*;[ \t]*\n"
)

mods = Path("src/modules")
# type name -> full module name (last wins if collision)
type_to_mod: dict[str, str] = {}
for p in mods.glob("carcer*.cppm"):
    if p.name == "carcer.cppm":
        continue
    stem = p.name[: -len(".cppm")]
    type_to_mod[stem.split(".")[-1]] = stem

fixed = 0
for p in mods.glob("carcer*.cppm"):
    if p.name == "carcer.cppm":
        continue
    own = p.name[: -len(".cppm")]
    own_type = own.split(".")[-1]
    t = p.read_text(encoding="utf-8")
    needed_imports: list[str] = []

    def repl(m: re.Match) -> str:
        name = m.group(1)
        if name == own_type:
            return m.group(0)
        mod = type_to_mod.get(name)
        if not mod or mod == own:
            return m.group(0)
        needed_imports.append(mod)
        return f"// modules: was fwd {name}; imported {mod}\n"

    t2 = FWD.sub(repl, t)
    if not needed_imports:
        if t2 != t:
            p.write_text(t2, encoding="utf-8", newline="\n")
            fixed += 1
        continue

    # dedupe imports
    uniq = []
    seen = set()
    for m in needed_imports:
        if m not in seen and f"import {m};" not in t2:
            seen.add(m)
            uniq.append(m)

    if uniq:
        lines = t2.splitlines(True)
        out = []
        inserted = False
        for line in lines:
            out.append(line)
            if not inserted and line.startswith("export module "):
                for m in uniq:
                    out.append(f"export import {m};\n")
                inserted = True
        t2 = "".join(out)

    p.write_text(t2, encoding="utf-8", newline="\n")
    fixed += 1

print("fixed", fixed)
