#!/usr/bin/env python3
"""Merge facade interfaces while moving non-exported bodies to implementation units.

This helper targets the source shape produced by the earlier consolidation
script: declarations are wrapped in `export { ... } // export` blocks and
out-of-class definitions follow those blocks in the same interface file.
"""
from __future__ import annotations

import argparse
import re
from pathlib import Path

from consolidate_module import (
    ROOT,
    SRC,
    dedupe_import_lines,
    display_path,
    import_target,
    iter_rewrite_files,
    rewrite_imports,
)


EXPORT_MODULE_RE = re.compile(r"^export module ([\w.:]+);", re.M)
EXPORT_BLOCK_RE = re.compile(r"^export \{\n.*?^\} // export\s*$", re.M | re.S)


def parse_source(path: Path) -> dict:
    text = path.read_text(encoding="utf-8")
    module_match = EXPORT_MODULE_RE.search(text)
    if not module_match:
        raise SystemExit(f"no export module declaration in {path}")

    name = module_match.group(1)
    head = text[: module_match.start()]
    rest = text[module_match.end() :]
    blocks = list(EXPORT_BLOCK_RE.finditer(rest))
    if not blocks:
        raise SystemExit(f"no marked export blocks in {path}")
    if rest.count("export {") != len(blocks):
        raise SystemExit(f"unrecognized export block in {path}")

    includes = re.findall(r"^#include .+$", head, re.M)
    preamble = rest[: blocks[0].start()]
    includes.extend(re.findall(r"^#include .+$", preamble, re.M))
    imports = [
        line.strip()
        for line in preamble.splitlines()
        if line.strip().startswith(("import ", "export import "))
    ]
    declarations = [match.group(0).strip() for match in blocks]
    implementation = EXPORT_BLOCK_RE.sub("", rest[blocks[0].start() :]).strip()
    return {
        "name": name,
        "path": path,
        "includes": includes,
        "imports": imports,
        "declarations": declarations,
        "implementation": implementation,
    }


def merge_imports(units: list[dict], new_name: str, for_implementation: bool) -> list[str]:
    old_names = {unit["name"] for unit in units}
    result: list[str] = []
    positions: dict[str, int] = {}
    for unit in units:
        for line in unit["imports"]:
            target = import_target(line, unit["name"])
            if target is None or target in old_names or target == new_name:
                continue
            normalized = f"import {target};" if for_implementation else line
            if target in positions:
                index = positions[target]
                if not for_implementation and line.startswith("export import "):
                    result[index] = f"export import {target};"
                continue
            positions[target] = len(result)
            result.append(normalized)
    if not for_implementation:
        result.sort(key=lambda line: (not line.startswith("export import "), line))
    return result


def split_includes(includes: list[str]) -> tuple[list[str], list[str]]:
    normal: list[str] = []
    macros: list[str] = []
    for include in includes:
        target = macros if "macros.h" in include else normal
        if include not in target:
            target.append(include)
    return normal, macros


def render_interface(units: list[dict], new_name: str) -> str:
    includes, macros = split_includes(
        [include for unit in units for include in unit["includes"]]
    )
    imports = merge_imports(units, new_name, for_implementation=False)
    declarations = [
        declaration
        for unit in units
        for declaration in unit["declarations"]
    ]
    lines = (
        ["module;"]
        + includes
        + ["", f"export module {new_name};"]
        + imports
        + macros
        + ["", "\n\n".join(declarations), ""]
    )
    return "\n".join(lines)


def render_implementation(unit: dict, units: list[dict], new_name: str) -> str:
    includes, macros = split_includes(unit["includes"])
    imports = merge_imports(units, new_name, for_implementation=True)
    lines = (
        ["module;"]
        + includes
        + ["", f"module {new_name};"]
        + imports
        + macros
        + ["", unit["implementation"], ""]
    )
    return "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--name", required=True)
    parser.add_argument("--out", required=True)
    parser.add_argument("--dry-run", action="store_true")
    parser.add_argument("sources", nargs="+")
    args = parser.parse_args()

    source_paths = [(ROOT / source).resolve() for source in args.sources]
    units = [parse_source(path) for path in source_paths]
    output = (ROOT / args.out).resolve()
    old_names = [unit["name"] for unit in units]
    interface = render_interface(units, args.name)
    implementations = {
        unit["path"].with_suffix(".cpp"): render_implementation(
            unit, units, args.name
        )
        for unit in units
    }

    print(
        f"merge {len(units)} interfaces -> {args.name}: "
        f"{len(interface.splitlines())} interface lines"
    )
    for path, text in implementations.items():
        print(f"  {display_path(path)}: {len(text.splitlines())} implementation lines")
    if args.dry_run:
        return 0

    output.write_text(interface, encoding="utf-8", newline="\n")
    for path, text in implementations.items():
        path.write_text(text, encoding="utf-8", newline="\n")

    source_set = {path.resolve() for path in source_paths}
    for path in iter_rewrite_files([SRC]):
        resolved = path.resolve()
        if resolved == output or resolved in source_set or resolved in {
            implementation.resolve() for implementation in implementations
        }:
            continue
        original = path.read_text(encoding="utf-8")
        rewritten = rewrite_imports(original, old_names, args.name)
        if rewritten != original:
            path.write_text(dedupe_import_lines(rewritten), encoding="utf-8", newline="\n")
            print(f"rewrite {display_path(path)}")

    for path in source_paths:
        if path != output:
            path.unlink()
            print(f"delete {display_path(path)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
