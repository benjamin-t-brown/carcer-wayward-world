#!/usr/bin/env python3
"""Fold a sibling implementation unit into its interface unit.

  python scripts/modules/fold_cpp_into_cppm.py src/ui/elements/buttons
  python scripts/modules/fold_cpp_into_cppm.py src/ui/components --skip ChCompactInfo --skip ListMagicSpells

Method bodies stay after `} // export` so they compile in the BMI object
without becoming implicitly-inline class members.
"""
from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
EXPORT_MOD_RE = re.compile(r"^export module ([\w.]+);", re.M)
MODULE_RE = re.compile(r"^module ([\w.]+);", re.M)
IMPORT_RE = re.compile(r"^(?:export\s+)?import\s+([\w.]+)\s*;")


def parse_unit(path: Path) -> dict:
    text = path.read_text(encoding="utf-8")
    if not text.startswith("module;"):
        raise SystemExit(f"expected global fragment in {path}")
    em = EXPORT_MOD_RE.search(text) or MODULE_RE.search(text)
    if not em:
        raise SystemExit(f"no module declaration in {path}")
    gfrag = text[len("module;") : em.start()]
    rest = text[em.end() :]
    imports: list[tuple[bool, str]] = []
    lines = rest.splitlines(keepends=True)
    i = 0
    has_macros = False
    while i < len(lines):
        raw = lines[i]
        s = raw.strip()
        if not s or s.startswith("//"):
            i += 1
            continue
        if s == '#include "macros.h"':
            has_macros = True
            i += 1
            continue
        m = IMPORT_RE.match(s)
        if m:
            imports.append((s.startswith("export import"), m.group(1)))
            i += 1
            continue
        break
    impl = "".join(lines[i:]).strip() + "\n"
    return {
        "name": em.group(1),
        "gfrag": gfrag,
        "imports": imports,
        "has_macros": has_macros,
        "impl": impl,
    }


def merge_gfrag(a: str, b: str) -> str:
    seen: set[str] = set()
    out: list[str] = []
    for block in (a, b):
        for line in block.splitlines():
            s = line.strip()
            if not s:
                continue
            if s in seen:
                continue
            seen.add(s)
            out.append(line.rstrip())
    return "\n".join(out) + ("\n" if out else "")


def merge_imports(
    iface: list[tuple[bool, str]], impl: list[tuple[bool, str]], own: str
) -> list[str]:
    exported: dict[str, bool] = {}
    order: list[str] = []
    for exp, tgt in iface + impl:
        if tgt == own:
            continue
        if tgt not in exported:
            order.append(tgt)
            exported[tgt] = exp
        elif exp:
            exported[tgt] = True
    return [
        f"{'export import' if exported[t] else 'import'} {t};" for t in order
    ]


def fold_pair(cppm: Path, cpp: Path) -> None:
    iface = parse_unit(cppm)
    impl = parse_unit(cpp)
    if iface["name"] != impl["name"]:
        raise SystemExit(f"module mismatch {cppm} vs {cpp}")
    if not iface["impl"].rstrip().endswith("} // export"):
        raise SystemExit(f"expected `}} // export` at end of {cppm}")
    if impl["impl"].startswith("export {"):
        raise SystemExit(f"{cpp} looks like an interface")

    gfrag = merge_gfrag(iface["gfrag"], impl["gfrag"])
    imports = merge_imports(iface["imports"], impl["imports"], iface["name"])

    out = ["module;\n", gfrag]
    if gfrag and not gfrag.endswith("\n"):
        out.append("\n")
    out.append(f"\nexport module {iface['name']};\n")
    for line in imports:
        out.append(line + "\n")
    if iface["has_macros"] or impl["has_macros"]:
        out.append('#include "macros.h"\n')
    out.append("\n")
    out.append(iface["impl"].rstrip() + "\n\n")
    out.append(impl["impl"])
    cppm.write_text("".join(out), encoding="utf-8", newline="\n")
    cpp.unlink()
    print("folded", cpp.relative_to(ROOT).as_posix(), "->", cppm.relative_to(ROOT).as_posix())


def collect_pairs(folder: Path, skip_stems: set[str]) -> list[tuple[Path, Path]]:
    pairs = []
    for cpp in sorted(folder.rglob("*.cpp")):
        if cpp.stem in skip_stems:
            print("skip", cpp.relative_to(ROOT).as_posix())
            continue
        cppm = cpp.with_suffix(".cppm")
        if cppm.exists():
            pairs.append((cppm, cpp))
    return pairs


def main() -> int:
    skip_stems: set[str] = set()
    dirs: list[str] = []
    argv = sys.argv[1:]
    i = 0
    while i < len(argv):
        if argv[i] == "--skip" and i + 1 < len(argv):
            skip_stems.add(argv[i + 1])
            i += 2
            continue
        dirs.append(argv[i])
        i += 1
    if not dirs:
        print(
            "usage: fold_cpp_into_cppm.py <dir> [...] [--skip StemName]",
            file=sys.stderr,
        )
        return 2
    for arg in dirs:
        folder = Path(arg)
        if not folder.is_absolute():
            folder = ROOT / folder
        pairs = collect_pairs(folder, skip_stems)
        if not pairs:
            raise SystemExit(f"no .cpp/.cppm pairs in {folder}")
        for cppm, cpp in pairs:
            fold_pair(cppm, cpp)
    return 0


if __name__ == "__main__":
    sys.exit(main())
