#!/usr/bin/env python3
"""Replace `import carcer;` in production implementation units with the
domain modules each file actually uses. Leaves main.cpp and tests alone.
"""
from __future__ import annotations

import re
import sys
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SRC = ROOT / "src"

EXPORT_MOD_RE = re.compile(r"^export module ([\w.]+);", re.M)
MODULE_RE = re.compile(r"^module ([\w.]+);", re.M)
NS_RE = re.compile(r"^namespace\s+([\w:]+)\s*\{")
TYPE_RE = re.compile(r"\b(?:class|struct|enum class|enum)\s+(\w+)")
FUNC_RE = re.compile(
    r"^(?:template\s*<[^;{]+>\s*)?(?:[\w:<>,\s\*&]+?)\s+(\w+)\s*\(",
)
BARE_FUNC_RE = re.compile(r"^(\w+)\s*\(")
CONST_RE = re.compile(
    r"\b(?:inline\s+)?constexpr\s+[\w:<>,\s\*&]+\s+(\w+)\s*="
)
QUAL_RE = re.compile(r"\b((?:[A-Za-z_]\w*::)+[A-Za-z_]\w*)")
IMPORT_RE = re.compile(r"^(?:export\s+)?import\s+([\w.]+)\s*;", re.M)

SKIP_NS = {
    "std",
    "bmin",
    "sdl2w",
    "glm",
    "nlohmann",
}

SPECIAL_NS_MODULE = {
    "strutil": "carcer.lib.StringUtil",
}

KEYWORDS = {
    "if",
    "for",
    "while",
    "switch",
    "catch",
    "return",
    "sizeof",
    "alignof",
    "static_assert",
    "defined",
}


def iter_cppm() -> list[Path]:
    out = []
    for p in SRC.rglob("*.cppm"):
        rel = p.relative_to(SRC).as_posix()
        if rel.startswith(("lib/sdl2w/", "lib/bmin/")):
            continue
        out.append(p)
    return out


def export_body(text: str) -> str:
    m = re.search(r"^export \{", text, re.M)
    if not m:
        return ""
    body = text[m.end() :]
    body = re.sub(r"\}\s*//\s*export\s*$", "", body.strip())
    return body


def index_types() -> dict[tuple[str, str], str]:
    """(namespace, name) -> module. namespace uses :: (e.g. state::actions)."""
    table: dict[tuple[str, str], str] = {}
    for p in iter_cppm():
        text = p.read_text(encoding="utf-8")
        em = EXPORT_MOD_RE.search(text)
        if not em:
            continue
        mod = em.group(1)
        if mod == "carcer":
            continue
        body = export_body(text)
        stack: list[str] = []  # "ns:ui" | "class" | "other"
        pending: str | None = None
        for raw in body.splitlines():
            s = raw.strip()
            ns_m = NS_RE.match(s)
            if ns_m:
                pending = "ns:" + ns_m.group(1)
            elif re.search(r"\b(?:class|struct)\s+\w+", s) and not s.endswith(";"):
                pending = "class"

            ns_parts: list[str] = []
            in_class = False
            for tag in stack:
                if tag.startswith("ns:"):
                    ns_parts.extend(tag[3:].split("::"))
                elif tag == "class":
                    in_class = True
            ns = "::".join(ns_parts)
            if ns and not in_class:
                for t in TYPE_RE.findall(s):
                    table[(ns, t)] = mod
                for name in CONST_RE.findall(s):
                    table[(ns, name)] = mod
                fm = FUNC_RE.match(s) or BARE_FUNC_RE.match(s)
                if fm:
                    name = fm.group(1)
                    if name not in KEYWORDS:
                        table[(ns, name)] = mod

            for ch in raw:
                if ch == "{":
                    stack.append(pending or "other")
                    pending = None
                elif ch == "}" and stack:
                    stack.pop()
    return table


def lookup(table: dict[tuple[str, str], str], qual: str) -> str | None:
    parts = qual.split("::")
    if not parts or parts[0] in SKIP_NS:
        return None
    if parts[0] in SPECIAL_NS_MODULE:
        return SPECIAL_NS_MODULE[parts[0]]
    for i in range(1, len(parts)):
        ns = "::".join(parts[:i])
        name = parts[i]
        if (ns, name) in table:
            return table[(ns, name)]
        for (tns, tname), mod in table.items():
            if tname == name and (tns == ns or tns.endswith("::" + ns)):
                return mod
    return None


def file_namespaces(cpp_text: str) -> set[str]:
    return {m.group(1) for m in re.finditer(r"^namespace\s+([\w:]+)\s*\{", cpp_text, re.M)}


def needed_modules(cpp_text: str, table: dict[tuple[str, str], str], own: str) -> list[str]:
    found: set[str] = set()
    for q in QUAL_RE.findall(cpp_text):
        mod = lookup(table, q)
        if mod and mod != own:
            found.add(mod)
    namespaces = file_namespaces(cpp_text)
    if namespaces:
        for (ns, name), mod in table.items():
            if mod == own or ns not in namespaces:
                continue
            if len(name) < 3 or name in KEYWORDS:
                continue
            if re.search(rf"\b{re.escape(name)}\b", cpp_text):
                found.add(mod)
    return sorted(found)


def existing_imports(text: str) -> set[str]:
    return set(IMPORT_RE.findall(text))


def apply_file(path: Path, table: dict[tuple[str, str], str]) -> bool:
    text = path.read_text(encoding="utf-8")
    mm = MODULE_RE.search(text)
    if not mm:
        return False
    own = mm.group(1)
    have = existing_imports(text)
    need = needed_modules(text, table, own)
    add = [m for m in need if m not in have and m != "carcer" and m != own]
    drop_umbrella = "import carcer;" in text
    if not add and not drop_umbrella:
        return False
    lines = text.splitlines(keepends=True)
    out: list[str] = []
    inserted = False
    for line in lines:
        if line.strip() == "import carcer;":
            if add and not inserted:
                for m in add:
                    out.append(f"import {m};\n")
                inserted = True
            continue
        out.append(line)
        if not inserted and add and MODULE_RE.match(line):
            for m in add:
                out.append(f"import {m};\n")
            inserted = True
    path.write_text("".join(out), encoding="utf-8", newline="\n")
    return True


def production_cpps() -> list[Path]:
    files = []
    for p in SRC.rglob("*.cpp"):
        rel = p.relative_to(SRC).as_posix()
        if rel.startswith(("lib/sdl2w/", "lib/bmin/", "__test__/")):
            continue
        if p.name == "main.cpp":
            continue
        files.append(p)
    return files


def main() -> int:
    table = index_types()
    print(f"indexed {len(table)} names")
    n = 0
    for p in production_cpps():
        if apply_file(p, table):
            n += 1
            print("narrowed", p.relative_to(ROOT).as_posix())
    print(f"updated {n} files")
    return 0


if __name__ == "__main__":
    sys.exit(main())
