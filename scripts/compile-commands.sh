#!/bin/bash
# Generate compile_commands.json for clangd with C++ modules support.
# Prefer clang++ (not g++ -fmodules-ts) so clangd can build its own BMIs.
# Re-run after adding/moving/deleting .cppm files, then restart clangd.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

cd "$ROOT_DIR"

if [[ -z "${CLANGXX:-}" ]]; then
  if command -v clang++ >/dev/null 2>&1; then
    CLANGXX="$(command -v clang++)"
  elif [[ -x /c/progs/msys2/ucrt64/bin/clang++.exe ]]; then
    CLANGXX=/c/progs/msys2/ucrt64/bin/clang++.exe
  elif [[ -x /ucrt64/bin/clang++.exe ]]; then
    CLANGXX=/ucrt64/bin/clang++.exe
  else
    CLANGXX=clang++
    echo "warning: clang++ not found; using 'clang++' as the driver name" >&2
  fi
fi

if [[ -z "${PYTHON:-}" ]]; then
  if command -v python >/dev/null 2>&1; then
    PYTHON=python
  elif command -v python3 >/dev/null 2>&1; then
    PYTHON=python3
  else
    echo "python not found on PATH" >&2
    exit 1
  fi
fi

COMPILER="$CLANGXX"
ROOT_JSON="$ROOT_DIR"
if command -v cygpath >/dev/null 2>&1; then
  COMPILER="$(cygpath -m "$CLANGXX")"
  ROOT_JSON="$(cygpath -m "$ROOT_DIR")"
fi

"$PYTHON" - "$ROOT_JSON" "$COMPILER" <<'PY'
import json
import sys
from pathlib import Path

root = Path(sys.argv[1]).resolve()
compiler = sys.argv[2].replace("\\", "/")
src = root / "src"
mod_dir = src / "modules"
sdl_mod = src / "lib" / "sdl2w" / "modules"
bmin_mod = sdl_mod / "bmin"

COMMON = [
    "-Wall",
    "-std=c++23",
    "-Isrc/modules",
]
if sdl_mod.is_dir():
    COMMON.append("-Isrc/lib/sdl2w/modules")
if bmin_mod.is_dir():
    COMMON.append("-Isrc/lib/sdl2w/modules/bmin")


def entry(source: Path) -> dict:
    rel = source.relative_to(root).as_posix()
    obj = ".cache/clangd-obj/" + rel.replace("/", "__") + ".o"
    args = [compiler, *COMMON]
    if source.suffix == ".cppm":
        args.extend(["-x", "c++-module"])
    args.extend(["-c", source.as_posix(), "-o", (root / obj).as_posix()])
    return {
        "directory": root.as_posix(),
        "arguments": args,
        "file": source.as_posix(),
    }


entries: list[dict] = []
seen: set[Path] = set()


def add(p: Path) -> None:
    if not p.is_file() or p in seen:
        return
    seen.add(p)
    entries.append(entry(p))


if sdl_mod.is_dir():
    for p in sorted(sdl_mod.glob("sdl2w*.cppm")):
        add(p)
if bmin_mod.is_dir():
    for p in sorted(bmin_mod.glob("bmin*.cppm")):
        add(p)

for p in sorted(src.rglob("*.cppm")):
    rel = p.relative_to(src).as_posix()
    if rel.startswith(("lib/sdl2w/", "lib/bmin/")):
        continue
    add(p)

for p in sorted(src.rglob("*.cpp")):
    rel = p.relative_to(src).as_posix()
    if rel.startswith(("lib/sdl2w/", "lib/bmin/", "modules/")):
        continue
    add(p)

out = root / "compile_commands.json"
out.write_text(json.dumps(entries, indent=2) + "\n", encoding="utf-8", newline="\n")
print(f"Wrote {out} ({len(entries)} entries)", file=sys.stderr)
print(f"Compiler driver: {compiler}", file=sys.stderr)
PY
