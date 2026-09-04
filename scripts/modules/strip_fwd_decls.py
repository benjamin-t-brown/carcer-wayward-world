#!/usr/bin/env python3
"""Strip exported forward-declarations that conflict with owning modules."""
from pathlib import Path
import re

FWD = re.compile(
    r"(?m)^[ \t]*(?:struct|class)[ \t]+([A-Za-z_][A-Za-z0-9_]*)[ \t]*;[ \t]*\n"
)

# Build set of type names that have their own modules (last segment of module name)
mods = Path("src/modules")
owners = set()
for p in mods.glob("carcer*.cppm"):
    if p.name == "carcer.cppm":
        continue
    # carcer.model.templates.CharacterTemplate.cppm -> CharacterTemplate
    stem = p.name[: -len(".cppm")]
    owners.add(stem.split(".")[-1])

fixed_files = 0
removed = 0
for p in mods.glob("carcer*.cppm"):
    if p.name == "carcer.cppm":
        continue
    t = p.read_text(encoding="utf-8")
    own = p.name[: -len(".cppm")].split(".")[-1]

    def repl(m: re.Match) -> str:
        global removed
        name = m.group(1)
        if name in owners and name != own:
            removed += 1
            return f"// modules: stripped fwd decl {name}\n"
        return m.group(0)

    t2 = FWD.sub(repl, t)
    if t2 != t:
        p.write_text(t2, encoding="utf-8", newline="\n")
        fixed_files += 1

print(f"files={fixed_files} removed={removed} owners={len(owners)}")
