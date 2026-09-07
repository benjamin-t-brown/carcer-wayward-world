#!/usr/bin/env python3
"""Enforce Carcer's permanent production include direction."""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

INCLUDE_RE = re.compile(r'^\s*#\s*include\s*"([^"]+)"')
SOURCE_SUFFIXES = {".h", ".hpp", ".cpp", ".cc"}
FORBIDDEN = {
    "model": {"db", "state", "actions", "ui", "layers"},
    "rules": {"state", "actions", "ui", "layers"},
    "state": {"actions", "ui", "layers"},
    "actions": {"ui", "layers"},
    "ui": {"layers"},
}


def subsystem(relative: Path) -> str | None:
    parts = relative.parts
    if not parts or parts[0] != "src" or "__test__" in parts:
        return None
    if len(parts) == 2 and parts[1] == "main.cpp":
        return "application"
    if len(parts) < 2:
        return None
    top = parts[1]
    if top in {"lib", "compat"}:
        return "foundation"
    if top == "data" or (top == "model" and len(parts) > 2 and parts[2] in {"templates", "stats"}):
        return "data"
    if top == "model":
        return "model"
    if top == "db":
        return "db"
    if top in {"game", "in3", "runner"}:
        return "rules"
    if top == "actions" or (top == "state" and len(parts) > 2 and parts[2] == "actions"):
        return "actions"
    if top == "state":
        return "state"
    if top == "ui":
        return "ui"
    if top == "layers":
        return "layers"
    return None


def resolve_include(root: Path, source: Path, spelling: str) -> Path | None:
    spelling = spelling.replace("\\", "/")
    if spelling.startswith("src/"):
        candidate = root / spelling
    else:
        relative_candidate = source.parent / spelling
        source_root_candidate = root / "src" / spelling
        candidate = relative_candidate if relative_candidate.exists() else source_root_candidate
    try:
        resolved = candidate.resolve(strict=True)
        resolved.relative_to(root)
        return resolved
    except (FileNotFoundError, ValueError):
        return None


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument(
        "--source-subsystem",
        action="append",
        choices=sorted(FORBIDDEN),
        help="check only this source subsystem (repeatable); default checks all",
    )
    args = parser.parse_args()
    root = args.root.resolve()
    requested_subsystems = set(args.source_subsystem or [])
    violations: list[tuple[str, str, str, str]] = []

    for source in sorted((root / "src").rglob("*")):
        if not source.is_file() or source.suffix not in SOURCE_SUFFIXES:
            continue
        source_rel = source.relative_to(root)
        source_system = subsystem(source_rel)
        if source_system not in FORBIDDEN:
            continue
        if requested_subsystems and source_system not in requested_subsystems:
            continue
        for line_number, line in enumerate(source.read_text(errors="replace").splitlines(), 1):
            match = INCLUDE_RE.match(line)
            if not match:
                continue
            target = resolve_include(root, source, match.group(1))
            if target is None:
                continue
            target_rel = target.relative_to(root)
            target_system = subsystem(target_rel)
            if target_system in FORBIDDEN[source_system]:
                violations.append(
                    (source_rel.as_posix(), target_rel.as_posix(), source_system, target_system)
                )

    if violations:
        print("Forbidden production include edges:", file=sys.stderr)
        for source, target, source_system, target_system in violations:
            print(f"  {source} -> {target} ({source_system} -> {target_system})", file=sys.stderr)
        return 1

    print("Production include architecture passes")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
