#!/usr/bin/env python3
"""Convert a folder (recursively) of one-class modules into partitions of one
folder module.

    python scripts/modules/partitionize_folder.py \
        --module carcer.ui.elements --dir src/ui/elements

Every `*.cppm` under <dir> (except the generated primary unit) becomes a
partition named after the LAST dotted segment of its current module name:

    export module carcer.ui.elements.Quad;   ->  export module carcer.ui.elements:Quad;
    export module carcer.ui.ButtonClose;     ->  export module carcer.ui.elements:ButtonClose;

Within the folder, `import <absorbed-module>;` becomes `import :<Part>;`
(the `export import` prefix is preserved). A primary interface unit
`<dir>/<basename>.cppm` is written that `export import`s every partition.
Sibling `.cpp` impl units get `module <target>;` and their self-imports dropped.
Repo-wide (incl. the umbrella), `import <absorbed-module>;` becomes
`import <target>;`, deduped.

Then run: python scripts/modules/gen_bmi_makefile.py
"""
from __future__ import annotations

import argparse
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SRC = ROOT / "src"

EXPORT_MOD_RE = re.compile(r"^export module (carcer[\w.]+)\s*;", re.M)
MODULE_RE = re.compile(r"^module (carcer[\w.]+)\s*;", re.M)


def dedupe_imports(text: str) -> str:
    out, seen = [], set()
    for line in text.splitlines(keepends=True):
        s = line.strip()
        if s.startswith(("import ", "export import ")):
            if s in seen:
                continue
            seen.add(s)
        out.append(line)
    return "".join(out)


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--module", required=True, help="target module, e.g. carcer.ui.elements")
    ap.add_argument("--dir", required=True, help="folder to absorb, e.g. src/ui/elements")
    ap.add_argument("--primary", help="primary .cppm basename (default: last dir segment)")
    ap.add_argument("--non-recursive", action="store_true",
                    help="only direct .cppm children of --dir (not sub-folders)")
    ap.add_argument("--exclude", default="",
                    help="comma-separated .cppm stems under --dir to leave as their own module")
    args = ap.parse_args()

    target = args.module
    d = (ROOT / args.dir).resolve()
    if not d.is_dir():
        raise SystemExit(f"not a dir: {d}")
    primary_path = d / f"{args.primary or d.name}.cppm"
    excluded = {s.strip() for s in args.exclude.split(",") if s.strip()}

    # 1. discover absorbed modules  ({old module name -> partition name}, files)
    absorbed: dict[str, str] = {}
    files: dict[str, Path] = {}
    globber = d.glob if args.non_recursive else d.rglob
    for cppm in sorted(globber("*.cppm")):
        if cppm == primary_path or cppm.stem in excluded:
            continue
        m = EXPORT_MOD_RE.search(cppm.read_text(encoding="utf-8"))
        if not m:
            raise SystemExit(f"{cppm}: no `export module`")
        name = m.group(1)
        part = name.split(".")[-1]
        if part in absorbed.values():
            other = next(k for k, v in absorbed.items() if v == part)
            raise SystemExit(f"partition name clash '{part}': {name} vs {other} -- rename one first")
        absorbed[name] = part
        files[name] = cppm
    if not absorbed:
        raise SystemExit("no .cppm found under " + str(d))
    print(f"{target}: absorbing {len(absorbed)} modules -> {primary_path.relative_to(ROOT).as_posix()}")

    # regex: `import <one-of-absorbed>;`  (captures indent + export? + the name)
    alt = "|".join(re.escape(n) for n in sorted(absorbed, key=len, reverse=True))
    imp_absorbed_re = re.compile(rf"^([ \t]*)(export import|import)\s+({alt})\s*;", re.M)

    def to_partition_import(txt: str) -> str:
        return imp_absorbed_re.sub(lambda m: f"{m.group(1)}{m.group(2)} :{absorbed[m.group(3)]};", txt)

    # 2. rewrite each absorbed .cppm in place
    for name, cppm in files.items():
        part = absorbed[name]
        text = cppm.read_text(encoding="utf-8")
        text = EXPORT_MOD_RE.sub(f"export module {target}:{part};", text, count=1)
        text = to_partition_import(text)
        cppm.write_text(text, encoding="utf-8", newline="\n")
        print(f"  :{part:<24} {cppm.relative_to(ROOT).as_posix()}")

    # 3. primary interface unit
    primary_path.write_text(
        f"export module {target};\n"
        + "".join(f"export import :{p};\n" for p in sorted(absorbed.values())),
        encoding="utf-8",
        newline="\n",
    )
    print(f"  primary  {primary_path.relative_to(ROOT).as_posix()}")

    # 4. impl units under <dir>
    for cpp in sorted(d.rglob("*.cpp")):
        text = cpp.read_text(encoding="utf-8")
        m = MODULE_RE.search(text)
        if not m or m.group(1) not in absorbed:
            continue
        new = MODULE_RE.sub(f"module {target};", text, count=1)
        new = imp_absorbed_re.sub("", new)  # impl unit implicitly imports the primary
        cpp.write_text(dedupe_imports(new), encoding="utf-8", newline="\n")
        print(f"  impl     {cpp.relative_to(ROOT).as_posix()}")

    # 5. repo-wide: `import <absorbed>;` -> `import <target>;`
    # skip only the files we just partitioned + the primary; other files under
    # <dir> (e.g. sub-folder modules not absorbed) still need the rewrite.
    partitioned = {f.resolve() for f in files.values()} | {primary_path.resolve()}
    imp_repo_re = re.compile(rf"^([ \t]*)(export import|import)\s+({alt})\s*;", re.M)
    changed = 0
    for p in SRC.rglob("*"):
        if p.suffix not in (".cpp", ".cppm", ".h", ".hpp"):
            continue
        rel = p.relative_to(SRC).as_posix()
        if rel.startswith(("lib/sdl2w/", "lib/bmin/")):
            continue
        if p.resolve() in partitioned:
            continue
        old = p.read_text(encoding="utf-8")
        if not imp_repo_re.search(old):
            continue
        new = dedupe_imports(imp_repo_re.sub(rf"\1\2 {target};", old))
        if new != old:
            p.write_text(new, encoding="utf-8", newline="\n")
            print(f"  ref      {rel}")
            changed += 1
    print(f"\n{changed} repo-wide files updated. next: python scripts/modules/gen_bmi_makefile.py")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
