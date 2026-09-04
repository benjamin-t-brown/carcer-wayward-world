#!/usr/bin/env python3
"""Probe GCC BMI sizes for import vs export-import of bmin."""
from pathlib import Path
import subprocess
import os

ROOT = Path(__file__).resolve().parents[2]
SRC = ROOT / "src"
PROBE = SRC / "_bmi_probe"
PROBE.mkdir(exist_ok=True)

BMIN = SRC / "lib/sdl2w/modules/bmin"
SDL = SRC / "lib/sdl2w/modules"
FLAGS = [
    "-Wall",
    "-std=c++23",
    "-g",
    "-fmodules-ts",
    f"-I{SDL.as_posix()}",
    f"-I{BMIN.as_posix()}",
    f"-I{PROBE.as_posix()}",
]


def run(cmd, cwd):
    print("+", " ".join(cmd))
    r = subprocess.run(cmd, cwd=cwd, capture_output=True, text=True)
    if r.returncode != 0:
        print(r.stdout)
        print(r.stderr)
        raise SystemExit(r.returncode)
    return r


def main():
    os.chdir(SRC)
    # ensure bmin BMIs exist
    run(
        ["make", "-f", "lib/sdl2w/modules/bmin/make/build-bmi.mk", f"BMIN_MOD={BMIN.as_posix()}", "-j1"],
        SRC,
    )

    cases = {
        "probe.exp_bmin": """module;
export module probe.exp_bmin;
export import bmin.containers;
export struct ProbeA { int x; bmin::DynArray<int> ys; };
""",
        "probe.imp_bmin": """module;
export module probe.imp_bmin;
import bmin.containers;
export struct ProbeB { int x; bmin::DynArray<int> ys; };
""",
        "probe.tiny": """module;
export module probe.tiny;
export struct ProbeC { int x; };
""",
    }
    for name, src in cases.items():
        (PROBE / f"{name}.cppm").write_text(src, encoding="utf-8", newline="\n")

    gcm = SRC / "gcm.cache"
    for name in cases:
        gcm_file = gcm / f"{name}.gcm"
        if gcm_file.exists():
            gcm_file.unlink()
        run(["g++", *FLAGS, "-c", f"_bmi_probe/{name}.cppm", "-o", f"_bmi_probe/{name}.o"], SRC)
        gcm_file = gcm / f"{name}.gcm"
        sz = gcm_file.stat().st_size if gcm_file.exists() else -1
        print(f"SIZE {name}.gcm = {sz} ({sz/1e6:.2f} MB)")

    # import probe.exp_bmin and use it
    (PROBE / "probe.user.cppm").write_text(
        """module;
export module probe.user;
export import probe.exp_bmin;
export int use_a(ProbeA a) { return a.x + (int)a.ys.size(); }
""",
        encoding="utf-8",
        newline="\n",
    )
    run(["g++", *FLAGS, "-c", "_bmi_probe/probe.user.cppm", "-o", "_bmi_probe/probe.user.o"], SRC)
    u = gcm / "probe.user.gcm"
    print(f"SIZE probe.user.gcm = {u.stat().st_size} ({u.stat().st_size/1e6:.2f} MB)")

    # consumer TU
    (PROBE / "main.cpp").write_text(
        """import probe.user;
int main() { ProbeA a; a.x = 1; return use_a(a); }
""",
        encoding="utf-8",
        newline="\n",
    )
    run(["g++", *FLAGS, "-c", "_bmi_probe/main.cpp", "-o", "_bmi_probe/main.o"], SRC)
    print("main import OK")


if __name__ == "__main__":
    main()
