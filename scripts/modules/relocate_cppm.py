#!/usr/bin/env python3
"""Move flat src/modules/carcer.*.cppm files next to their .cpp / old-header paths.

Module *names* stay the same. Implementation units stay .cpp.
Idempotent: already-colocated files are left alone.

  python scripts/modules/relocate_cppm.py
  python scripts/modules/relocate_cppm.py --dry-run
"""
from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SRC = ROOT / "src"
MODULES = SRC / "modules"

FROM_RE = re.compile(r"^// --- from (.+?) ---", re.M)
EXPORT_RE = re.compile(r"^export module (carcer[\w.]*);", re.M)

# Thin aliases / merged units that have no `// --- from` comment.
ALIAS_DEST = {
    "carcer.state.core": "state/core.cppm",
    "carcer.state.StateManager": "state/StateManager.cppm",
    "carcer.state.StateManagerInterface": "state/StateManagerInterface.cppm",
    "carcer.state.AbstractAction": "state/AbstractAction.cppm",
    "carcer.state.ActionBus": "state/ActionBus.cppm",
    "carcer.state.UiManager": "state/UiManager.cppm",
    "carcer.ui.ListMagicSpells": "ui/components/lists/ListMagicSpells.cppm",
}


def dest_for(path: Path, text: str) -> Path:
    if path.name == "carcer.cppm":
        return MODULES / "carcer.cppm"

    m = FROM_RE.search(text)
    if m:
        orig = m.group(1).replace("\\", "/")
        if orig.endswith(".h") or orig.endswith(".hpp"):
            orig = orig.rsplit(".", 1)[0] + ".cppm"
        elif not orig.endswith(".cppm"):
            orig = orig + ".cppm"
        return SRC / orig

    em = EXPORT_RE.search(text)
    if not em:
        raise SystemExit(f"no export module / from-comment in {path}")
    mod = em.group(1)
    if mod in ALIAS_DEST:
        return SRC / ALIAS_DEST[mod]
    raise SystemExit(f"no destination mapping for {path.name} ({mod})")


def plan_moves() -> list[tuple[Path, Path]]:
    moves: list[tuple[Path, Path]] = []
    seen_dest: dict[Path, Path] = {}
    for p in sorted(MODULES.glob("carcer*.cppm")):
        text = p.read_text(encoding="utf-8")
        dest = dest_for(p, text)
        if dest in seen_dest:
            raise SystemExit(f"collision: {seen_dest[dest]} and {p} -> {dest}")
        seen_dest[dest] = p
        if p.resolve() != dest.resolve():
            moves.append((p, dest))
    return moves


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--dry-run", action="store_true")
    args = ap.parse_args()

    moves = plan_moves()
    if not moves:
        print("already colocated; nothing to move")
        return 0

    for src, dest in moves:
        rel_src = src.relative_to(ROOT).as_posix()
        rel_dest = dest.relative_to(ROOT).as_posix()
        print(f"{rel_src} -> {rel_dest}")
        if args.dry_run:
            continue
        dest.parent.mkdir(parents=True, exist_ok=True)
        if dest.exists():
            raise SystemExit(f"refusing to overwrite {dest}")
        src.rename(dest)

    print(f"{'would move' if args.dry_run else 'moved'} {len(moves)} files")
    return 0


if __name__ == "__main__":
    sys.exit(main())
