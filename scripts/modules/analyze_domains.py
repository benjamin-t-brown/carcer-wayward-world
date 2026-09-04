#!/usr/bin/env python3
from pathlib import Path
import re

SRC = Path(__file__).resolve().parents[2] / "src"
INC = re.compile(r'^\s*#\s*include\s*"([^"]+)"', re.M)


def domain(rel: str) -> str:
    p = rel.split("/")
    if p[0] == "lib":
        return "lib"
    if p[0] == "model":
        return "model"
    if p[0] == "db":
        return "db"
    if p[0] == "game":
        return "game"
    if p[0] == "runner":
        return "runner"
    if p[0] == "state":
        return "state.actions" if "actions" in p else "state"
    if p[0] == "ui":
        return "ui.observers" if "observers" in p else "ui"
    if p[0] == "layers":
        return "layers"
    return "other"


def main() -> None:
    edges = set()
    counts = {}
    headers = list(SRC.rglob("*.h")) + list(SRC.rglob("*.hpp"))
    for h in headers:
        rel = h.relative_to(SRC).as_posix()
        if rel.startswith(("lib/sdl2w", "lib/bmin", "__test__", "modules")):
            continue
        d = domain(rel)
        counts[d] = counts.get(d, 0) + 1
        text = h.read_text(encoding="utf-8", errors="replace")
        for m in INC.finditer(text):
            inc = m.group(1)
            if inc.startswith(("bmin/", "sdl2w/", "SDL")):
                continue
            for c in (SRC / inc, h.parent / inc):
                if c.is_file():
                    ir = c.relative_to(SRC).as_posix()
                    di = domain(ir)
                    if di != d and di != "other":
                        edges.add((d, di))
                    break
    print("counts", sorted(counts.items()))
    print("edges")
    for a, b in sorted(edges):
        print(f"  {a} -> {b}")


if __name__ == "__main__":
    main()
