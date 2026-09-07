#!/usr/bin/env python3
"""Merge several interface units into one domain module.

Example (layers):

  python scripts/modules/consolidate_module.py \\
      --name carcer.layers \\
      --out src/layers/_layers.cppm \\
      src/layers/Layer.cppm src/layers/LayerManager.cppm src/layers/ui/*.cppm

Then: python scripts/modules/gen_bmi_makefile.py
"""
from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SRC = ROOT / "src"

EXPORT_MOD_RE = re.compile(r"^export module ([\w.:]+);", re.M)
MODULE_RE = re.compile(r"^module ([\w.:]+);", re.M)
EXPORT_BRACE_RE = re.compile(r"^export \{", re.M)


def display_path(path: Path) -> str:
    try:
        return path.relative_to(ROOT).as_posix()
    except ValueError:
        return path.as_posix()


def parse_cppm(path: Path) -> dict:
    text = path.read_text(encoding="utf-8")
    m = EXPORT_MOD_RE.search(text)
    if not m:
        raise SystemExit(f"no export module in {path}")
    name = m.group(1)
    head, rest = text[: m.start()], text[m.end() :]
    includes = re.findall(r"^#include .+$", head, re.M)
    em = EXPORT_BRACE_RE.search(rest)
    if em:
        preamble, body = rest[: em.start()], rest[em.start() :].strip()
    else:
        # Primary aggregate units often contain only `export import`
        # declarations. They still need to participate so their consumers are
        # rewritten to the consolidated boundary.
        preamble, body = rest, ""
    imports: list[str] = []
    for line in preamble.splitlines():
        s = line.strip()
        if s.startswith("import ") or s.startswith("export import "):
            imports.append(s)
        elif s.startswith("#include"):
            includes.append(s)
    return {"name": name, "path": path, "includes": includes, "imports": imports, "body": body}


def import_target(line: str, owner: str | None = None) -> str | None:
    m = re.search(r"import ([\w.:]+)\s*;", line)
    if not m:
        return None
    target = m.group(1)
    if target.startswith(":") and owner:
        return owner.split(":", 1)[0] + target
    return target


def merge_units(units: list[dict], new_name: str) -> str:
    merged_names = {u["name"] for u in units}
    includes: list[str] = []
    seen_inc: set[str] = set()
    imports: list[str] = []
    seen_imp: set[str] = set()
    bodies: list[str] = []

    def add_include(inc: str) -> None:
        if inc not in seen_inc:
            seen_inc.add(inc)
            includes.append(inc)

    def add_import(line: str, owner: str) -> None:
        tgt = import_target(line, owner)
        if tgt is None or tgt in merged_names or tgt == new_name:
            return
        key = tgt
        if key in seen_imp:
            if line.startswith("export import "):
                imports[:] = [
                    f"export import {tgt};" if import_target(x) == tgt else x for x in imports
                ]
            return
        seen_imp.add(key)
        imports.append(line)

    for u in units:
        for inc in u["includes"]:
            add_include(inc)
        for imp in u["imports"]:
            add_import(imp, u["name"])
        bodies.append(u["body"].rstrip())

    export_imps = [x for x in imports if x.startswith("export import ")]
    plain_imps = [x for x in imports if not x.startswith("export import ")]
    macros = [i for i in includes if "macros.h" in i]
    other = [i for i in includes if "macros.h" not in i]
    gmf = ["module;"] + other
    lines = (
        gmf
        + ["", f"export module {new_name};"]
        + export_imps
        + plain_imps
        + macros
        + ["", "\n\n".join(body for body in bodies if body), ""]
    )
    return "\n".join(lines)


def rewrite_imports(text: str, old_names: list[str], new_name: str) -> str:
    owner_match = re.search(r"^(?:export )?module ([\w.:]+);", text, re.M)
    owner_base = owner_match.group(1).split(":", 1)[0] if owner_match else None
    names = sorted(old_names, key=len, reverse=True)
    for old in names:
        if owner_base and ":" in old and old.split(":", 1)[0] == owner_base:
            relative = ":" + old.split(":", 1)[1]
            text = re.sub(
                rf"(?<=import ){re.escape(relative)}(?=\s*;)", new_name, text
            )
        text = re.sub(rf"(?<![\w.:]){re.escape(old)}(?![\w.:])", new_name, text)
    return text


def dedupe_import_lines(text: str) -> str:
    out: list[str] = []
    seen: set[str] = set()
    for line in text.splitlines(keepends=True):
        stripped = line.strip()
        if stripped.startswith("import ") or stripped.startswith("export import "):
            if stripped in seen:
                continue
            seen.add(stripped)
        out.append(line)
    return "".join(out)


def iter_rewrite_files(roots: list[Path]) -> list[Path]:
    files: list[Path] = []
    for root in roots:
        for p in root.rglob("*"):
            if not p.is_file():
                continue
            rel = p.relative_to(root).as_posix()
            if rel.startswith(("lib/sdl2w/", "lib/bmin/")):
                continue
            if p.suffix not in {".cpp", ".cppm", ".h", ".hpp"}:
                continue
            files.append(p)
    return files


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--name", required=True, help="new module name, e.g. carcer.layers")
    ap.add_argument("--out", required=True, help="output .cppm path relative to repo or src")
    ap.add_argument("sources", nargs="+", help="interface .cppm files to merge")
    ap.add_argument(
        "--rewrite-root",
        action="append",
        default=[],
        help="additional source tree whose imports should be migrated",
    )
    ap.add_argument("--dry-run", action="store_true")
    args = ap.parse_args()

    sources = []
    for spec in args.sources:
        p = Path(spec)
        if not p.is_absolute():
            p = (ROOT / spec) if (ROOT / spec).exists() else (Path.cwd() / spec)
        if p.is_dir():
            sources.extend(sorted(p.rglob("*.cppm")))
        else:
            sources.append(p)
    sources = [p.resolve() for p in sources if p.suffix == ".cppm"]
    if not sources:
        raise SystemExit("no source .cppm files")

    out = Path(args.out)
    if not out.is_absolute():
        out = ROOT / args.out if not args.out.startswith("src/") else ROOT / args.out
        if not str(out).replace("\\", "/").endswith(".cppm"):
            raise SystemExit("--out must be a .cppm path")
    out = out.resolve()

    units = [parse_cppm(p) for p in sources]
    old_names = [u["name"] for u in units]
    merged = merge_units(units, args.name)

    print(f"merge {len(units)} units -> {args.name}")
    for u in units:
        print(f"  {u['name']}")

    if args.dry_run:
        print(merged[:500], "...")
        return 0

    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(merged, encoding="utf-8", newline="\n")

    impl_stems = {u["path"].with_suffix(".cpp") for u in units}
    for cpp in impl_stems:
        if not cpp.exists():
            continue
        text = cpp.read_text(encoding="utf-8")
        text = MODULE_RE.sub(f"module {args.name};", text, count=1)
        text = rewrite_imports(text, old_names, args.name)
        # Implementation units cannot import their own module.
        lines = []
        for line in text.splitlines(keepends=True):
            s = line.strip()
            if s in (f"import {args.name};", f"export import {args.name};"):
                continue
            lines.append(line)
        cpp.write_text("".join(lines), encoding="utf-8", newline="\n")
        print(f"impl {display_path(cpp)}")

    rewrite_roots = [SRC] + [Path(root).resolve() for root in args.rewrite_root]
    for p in iter_rewrite_files(rewrite_roots):
        if p.resolve() == out:
            continue
        if p.resolve() in {u["path"].resolve() for u in units}:
            continue
        orig = p.read_text(encoding="utf-8")
        text = rewrite_imports(orig, old_names, args.name)
        if text != orig:
            p.write_text(dedupe_import_lines(text), encoding="utf-8", newline="\n")
            print(f"rewrite {display_path(p)}")

    for u in units:
        if u["path"].resolve() != out:
            u["path"].unlink()
            print(f"delete {display_path(u['path'])}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
