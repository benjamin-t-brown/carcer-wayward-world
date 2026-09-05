#!/usr/bin/env python3
"""Regenerate modules/make/build-bmi.mk from module/partition edges in .cppm files.

Understands C++20 module partitions:
  * `export module carcer.ui.elements:Quad;`  -> unit "carcer.ui.elements:Quad"
    belonging to module "carcer.ui.elements".
  * `import :Quad;` inside that module -> depends on "carcer.ui.elements:Quad".
  * the primary interface unit `export module carcer.ui.elements;` depends on
    every partition of its module.

BMI object files replace ':' with '-' (":": illegal on Windows, matches the
wasm .pcm naming convention).
"""
from __future__ import annotations

from collections import deque
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[2]
SRC = ROOT / "src"
MODULES = SRC / "modules"

# cross-module import: `import carcer.foo;` or `import carcer.foo:part;`
IMPORT_RE = re.compile(r"^\s*(?:export\s+)?import\s+(carcer[\w.]+(?::\w+)?)\s*;", re.M)
# same-module partition import: `import :part;`
IMPORT_PART_RE = re.compile(r"^\s*(?:export\s+)?import\s+:(\w+)\s*;", re.M)
EXPORT_RE = re.compile(r"^export module (carcer[\w.]*(?::\w+)?)\s*;", re.M)


def objname(unit: str) -> str:
    return unit.replace(":", "-")


def iter_carcer_cppm() -> list[Path]:
    out: list[Path] = []
    for p in SRC.rglob("*.cppm"):
        rel = p.relative_to(SRC).as_posix()
        if rel.startswith(("lib/sdl2w/", "lib/bmin/")):
            continue
        out.append(p)
    return sorted(out)


def module_name(path: Path, text: str) -> str:
    m = EXPORT_RE.search(text)
    if not m:
        raise SystemExit(f"no export module in {path}")
    return m.group(1)


cppms = iter_carcer_cppm()
unit_file: dict[str, str] = {}  # unit name (may contain ':') -> path relative to src/
umbrella_rel = "modules/_carcer.cppm"

for p in cppms:
    rel = p.relative_to(SRC).as_posix()
    text = p.read_text(encoding="utf-8")
    name = module_name(p, text)
    if name == "carcer":
        umbrella_rel = rel
        continue
    if name in unit_file:
        raise SystemExit(f"duplicate export module {name}: {unit_file[name]} and {rel}")
    unit_file[name] = rel

deps: dict[str, set[str]] = {u: set() for u in unit_file}
for unit, rel in unit_file.items():
    mod = unit.split(":", 1)[0]
    text = (SRC / rel).read_text(encoding="utf-8")
    for m in IMPORT_RE.finditer(text):
        other = m.group(1)
        if other in unit_file and other != unit:
            deps[unit].add(other)
    for m in IMPORT_PART_RE.finditer(text):
        sibling = f"{mod}:{m.group(1)}"
        if sibling in unit_file and sibling != unit:
            deps[unit].add(sibling)

# primary interface unit of a module depends on all its partitions
for unit in unit_file:
    if ":" in unit:
        primary = unit.split(":", 1)[0]
        if primary in unit_file:
            deps[primary].add(unit)

indeg = {u: 0 for u in unit_file}
adj: dict[str, list[str]] = {u: [] for u in unit_file}
for u, ds in deps.items():
    for d in ds:
        adj[d].append(u)
        indeg[u] += 1

q = deque(sorted([u for u, i in indeg.items() if i == 0]))
order: list[str] = []
while q:
    n = q.popleft()
    order.append(n)
    for u in sorted(adj[n]):
        indeg[u] -= 1
        if indeg[u] == 0:
            q.append(u)

if len(order) != len(unit_file):
    leftover = [u for u in sorted(unit_file) if u not in order]
    print("WARNING cycle involving", leftover[:10], "... appending")
    order.extend(leftover)

lines = [
    "CXX ?= g++",
    "CARCER_MOD ?= modules",
    "SDL2W_MOD ?= lib/sdl2w/modules",
    "BMIN_MOD ?= lib/sdl2w/modules/bmin",
    "FLAGS = -Wall -std=c++23 -g -fmodules-ts -I$(CARCER_MOD) -I$(SDL2W_MOD) -I$(BMIN_MOD)",
    "OBJDIR = .carcer-bmi",
    "",
    ".PHONY: all clean",
    "",
    "all: $(OBJDIR)/carcer.o",
    "\t@mkdir -p gcm.cache",
    "\t@touch gcm.cache/.carcer-ready",
    "",
    "$(OBJDIR):",
    "\t@mkdir -p $@",
    "",
]

objs = []
for unit in order:
    obj = f"$(OBJDIR)/{objname(unit)}.o"
    objs.append(obj)
    dep_objs = " ".join(f"$(OBJDIR)/{objname(d)}.o" for d in sorted(deps[unit]))
    rel = unit_file[unit]
    prereq = rel
    if dep_objs:
        prereq += " " + dep_objs
    lines.append(f"{obj}: {prereq} | $(OBJDIR)")
    lines.append(f"\t$(CXX) $(FLAGS) -c {rel} -o $@")
    lines.append("")

lines.append(f"$(OBJDIR)/carcer.o: {umbrella_rel} {' '.join(objs)} | $(OBJDIR)")
lines.append(f"\t$(CXX) $(FLAGS) -c {umbrella_rel} -o $@")
lines.append("")
lines.append("clean:")
lines.append("\trm -rf $(OBJDIR)")
lines.append("")
lines.append("CARCER_BMI_OBJ_LIST = " + " ".join(objs) + " $(OBJDIR)/carcer.o")
lines.append("")

out = MODULES / "make" / "build-bmi.mk"
out.write_text("\n".join(lines) + "\n", encoding="utf-8", newline="\n")
(MODULES / "bmi_objs.list").write_text(
    "\n".join([f".carcer-bmi/{objname(u)}.o" for u in order] + [".carcer-bmi/carcer.o", ""]),
    encoding="utf-8",
    newline="\n",
)
(MODULES / "module_order.txt").write_text(
    "\n".join(order) + "\n",
    encoding="utf-8",
    newline="\n",
)
iface_list = [unit_file[u] for u in order] + [umbrella_rel]
(MODULES / "cppm_sources.list").write_text(
    "\n".join(iface_list) + "\n",
    encoding="utf-8",
    newline="\n",
)
print(f"wrote {out} units={len(order)}")
