#!/usr/bin/env python3
"""
Convert Carcer headers to named modules.

One-shot: do not re-run for day-to-day work. Writes flat src/modules/carcer.*.cppm;
colocate with `python scripts/modules/relocate_cppm.py` then
`python scripts/modules/gen_bmi_makefile.py`.

One module per header when the include+fwd-decl graph is acyclic.
Strongly connected components (include cycles OR mutual fwd-decl API deps)
are amalgamated into a single module so GCC does not assign the same type
to two modules.
"""

from __future__ import annotations

import re
import sys
from collections import defaultdict, deque
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SRC = ROOT / "src"
MODULES = SRC / "modules"

INCLUDE_RE = re.compile(r'^\s*#\s*include\s*([<"])([^>"]+)[>"]', re.M)
PRAGMA_ONCE_RE = re.compile(r"^\s*#\s*pragma\s+once\s*\n?", re.M)
FWD_LINE_RE = re.compile(
    r"^\s*(?:inline\s+)?(?:struct|class|enum\s+class|enum)\s+([A-Za-z_]\w*)\s*;\s*$"
)
TYPE_DEF_RE = re.compile(
    r"\b(?:struct|class|enum\s+class|enum)\s+([A-Za-z_]\w*)\s*[:{]"
)
FWD_ANY_RE = re.compile(
    r"\b(?:struct|class|enum\s+class|enum)\s+([A-Za-z_]\w*)\s*;"
)

def gmf_for(*texts: str) -> str:
    """Build a minimal global module fragment from scanned source text."""
    text = "\n".join(texts)
    incs: list[str] = []

    def need(*headers: str) -> None:
        for h in headers:
            if h not in incs:
                incs.append(h)

    # Always useful for size_t / fixed ints in game code
    need("cstddef", "cstdint", "utility")

    checks = [
        (("std::string_view", "string_view"), ("string_view",)),
        (("std::string",), ("string",)),
        (("std::optional",), ("optional",)),
        (("std::vector",), ("vector",)),
        (("std::array",), ("array",)),
        (("std::deque",), ("deque",)),
        (("std::variant",), ("variant",)),
        (("std::function",), ("functional",)),
        (("std::unique_ptr", "std::shared_ptr", "std::make_unique", "std::make_shared"), ("memory",)),
        (("std::runtime_error", "std::logic_error", "std::exception", "std::invalid_argument"), ("stdexcept",)),
        (("std::ostream", "std::istream", "operator<<"), ("ostream",)),
        (("std::stringstream", "std::ostringstream", "std::istringstream"), ("sstream",)),
        (("std::min(", "std::max(", "std::sort", "std::find", "std::move(", "std::swap", "std::clamp"), ("algorithm",)),
        (("std::abs", "std::sqrt", "std::floor", "std::ceil", "std::round", "std::pow", "std::sin", "M_PI"), ("cmath",)),
        (("std::numeric_limits",), ("limits",)),
        (("printf", "sprintf", "snprintf", "sscanf", "FILE", "stdout", "stderr"), ("cstdio",)),
        (("std::memcpy", "std::memset", "std::strlen", "std::strcmp", "std::strcpy"), ("cstring",)),
        (("std::isdigit", "std::isspace", "std::tolower", "std::toupper"), ("cctype",)),
        (("std::time", "std::clock", "CLOCKS_PER_SEC", "time_t"), ("ctime",)),
        (("std::atoi", "std::atof", "std::malloc", "std::free", "std::exit", "std::abort", "NULL", "std::rand", "std::srand"), ("cstdlib",)),
        (("std::type_info", "typeid", "std::type_index"), ("typeinfo", "typeindex")),
        (("assert(",), ("cassert",)),
        (("std::from_chars", "std::to_chars"), ("charconv",)),
        (("std::error_code", "std::errc"), ("system_error",)),
        (("abi::__",), ("cxxabi.h",)),
    ]
    for needles, headers in checks:
        if any(n in text for n in needles):
            need(*headers)

    # SDL only when touching pixels / raw SDL types (sdl2w import covers most needs)
    needs_sdl_hdr = any(
        x in text
        for x in (
            "SDL_Color",
            "SDL_Rect",
            "SDL_Event",
            "SDL_Keycode",
            "SDL_Renderer",
            "SDL_Window",
            "SDL_Texture",
            "SDL_PIXELFORMAT",
            "SdlPixels",
        )
    )

    lines = ["module;"]
    for h in incs:
        if h.endswith(".h"):
            lines.append(f"#include <{h}>")
        else:
            lines.append(f"#include <{h}>")
    if needs_sdl_hdr:
        lines += [
            "#if __has_include(<SDL.h>)",
            "#include <SDL.h>",
            "#include <SDL_pixels.h>",
            "#else",
            "#include <SDL2/SDL.h>",
            "#include <SDL2/SDL_pixels.h>",
            "#endif",
        ]
    return "\n".join(lines)


def read(p: Path) -> str:
    return p.read_text(encoding="utf-8", errors="replace")


def write(p: Path, t: str) -> None:
    p.parent.mkdir(parents=True, exist_ok=True)
    p.write_text(t.replace("\r\n", "\n"), encoding="utf-8", newline="\n")


def is_src(rel: str) -> bool:
    if rel.startswith(("lib/sdl2w/", "lib/bmin/", "__test__/", "modules/", "_bmi_probe/")):
        return False
    return rel.startswith(
        ("lib/", "model/", "db/", "game/", "runner/", "state/", "ui/", "layers/")
    )


# Deep dotted names trip GCC BMI (ICE / Bad file data). Flatten selected paths.
_SHORT_MODULE_PREFIXES = (
    ("ui/components/lists/", "carcer.ui."),
    ("ui/components/borders/", "carcer.ui."),
    ("ui/elements/buttons/", "carcer.ui."),
)


def mod_name_for_rel(rel: str) -> str:
    stem = rel.rsplit(".", 1)[0]
    for prefix, short in _SHORT_MODULE_PREFIXES:
        if stem.startswith(prefix):
            return short + stem[len(prefix) :].replace("/", ".")
    return "carcer." + stem.replace("/", ".")


def collect():
    hs, ss = [], []
    for p in SRC.rglob("*"):
        if not p.is_file():
            continue
        rel = p.relative_to(SRC).as_posix()
        if not is_src(rel):
            continue
        if p.suffix in {".h", ".hpp"}:
            hs.append(p)
        elif p.suffix == ".cpp":
            ss.append(p)
    return hs, ss


def resolve(inc: str, frm: Path) -> Path | None:
    for c in (SRC / inc, frm.parent / inc):
        if c.is_file():
            return c.resolve()
    return None


def includes(h: Path) -> list[Path]:
    out = []
    for m in INCLUDE_RE.finditer(read(h)):
        if m.group(1) != '"':
            continue
        name = m.group(2)
        if name.startswith(("bmin/", "sdl2w/", "SDL")):
            continue
        r = resolve(name, h)
        if r:
            out.append(r)
    return out


def strip_body(text: str) -> str:
    text = PRAGMA_ONCE_RE.sub("", text)
    text = INCLUDE_RE.sub("", text)
    text = re.sub(r"namespace sdl2w \{\s*\n\s*class Window;\s*\n\}", "", text)
    text = text.replace("  std::optional<sdl2w::Animation> animation;\n", "")
    text = text.replace(
        '    sdl2w::Logger().get(sdl2w::WARN) << "AbstractAction::act() called noop";',
        "    /* noop */",
    )
    if "enum class ScrollDirection { Up, Down };" in text:
        text = text.replace(
            "enum class ScrollDirection { Up, Down };",
            "enum class HeldScrollDirection { Up, Down };",
        )
        text = text.replace("ScrollDirection direction);", "HeldScrollDirection direction);")
    return re.sub(r"\n{3,}", "\n\n", text).strip() + "\n"


def defined_types(text: str) -> set[str]:
    return set(TYPE_DEF_RE.findall(text))


def split_fwds(body: str, foreign: set[str]) -> tuple[str, str]:
    """Move forward decls of foreign types out of the export body, preserving namespaces."""
    non_export: list[str] = []
    kept: list[str] = []
    ns_stack: list[tuple[str, int]] = []
    depth = 0
    for line in body.splitlines(True):
        stripped = line.strip()
        ns_open = re.match(r"^namespace\s+([A-Za-z_]\w*)\s*\{\s*$", stripped)
        if ns_open:
            depth += 1
            ns_stack.append((ns_open.group(1), depth))
            kept.append(line)
            continue
        m = FWD_LINE_RE.match(line)
        if m and m.group(1) in foreign:
            decl = stripped
            names = [n for n, _ in ns_stack]
            if names:
                block = "".join(f"namespace {n} {{\n" for n in names)
                block += f"  {decl}\n"
                block += "".join("}\n" for _ in names)
                non_export.append(block)
            else:
                non_export.append(decl + "\n")
            continue
        kept.append(line)
        depth += line.count("{") - line.count("}")
        if depth < 0:
            depth = 0
        while ns_stack and ns_stack[-1][1] > depth:
            ns_stack.pop()
    text = "".join(kept)
    prev = None
    while prev != text:
        prev = text
        text = re.sub(
            r"namespace\s+[A-Za-z_]\w*\s*\{\s*\}\s*(?://[^\n]*)?\n?",
            "",
            text,
        )
    return "".join(non_export), text


def sccs(nodes: list[Path], edges: dict[Path, set[Path]]) -> list[list[Path]]:
    index = 0
    stack: list[Path] = []
    onstack: set[Path] = set()
    indices: dict[Path, int] = {}
    lowlink: dict[Path, int] = {}
    result: list[list[Path]] = []

    def strongconnect(v: Path) -> None:
        nonlocal index
        indices[v] = index
        lowlink[v] = index
        index += 1
        stack.append(v)
        onstack.add(v)
        for w in edges.get(v, ()):
            if w not in indices:
                strongconnect(w)
                lowlink[v] = min(lowlink[v], lowlink[w])
            elif w in onstack:
                lowlink[v] = min(lowlink[v], indices[w])
        if lowlink[v] == indices[v]:
            comp = []
            while True:
                w = stack.pop()
                onstack.remove(w)
                comp.append(w)
                if w == v:
                    break
            result.append(comp)

    for v in nodes:
        if v not in indices:
            strongconnect(v)
    return result


def topo_units(units: list[int], unit_deps: dict[int, set[int]]) -> list[int]:
    indeg = {u: 0 for u in units}
    adj = defaultdict(list)
    for a, ds in unit_deps.items():
        for b in ds:
            if b in indeg and b != a:
                adj[b].append(a)
                indeg[a] += 1
    q = deque(sorted(u for u, d in indeg.items() if d == 0))
    out = []
    while q:
        n = q.popleft()
        out.append(n)
        for m in sorted(adj[n]):
            indeg[m] -= 1
            if indeg[m] == 0:
                q.append(m)
    if len(out) != len(units):
        out.extend([u for u in units if u not in out])
    return out


def topo_within(headers: list[Path], edge_map: dict[Path, set[Path]]) -> list[Path]:
    s = set(headers)
    indeg = {h: 0 for h in headers}
    adj = defaultdict(list)
    for h in headers:
        for d in edge_map.get(h, ()):
            if d in s:
                adj[d].append(h)
                indeg[h] += 1
    q = deque(sorted([h for h in headers if indeg[h] == 0], key=lambda x: x.as_posix()))
    out = []
    while q:
        n = q.popleft()
        out.append(n)
        for m in sorted(adj[n], key=lambda x: x.as_posix()):
            indeg[m] -= 1
            if indeg[m] == 0:
                q.append(m)
    if len(out) != len(headers):
        out.extend([h for h in sorted(headers, key=lambda x: x.as_posix()) if h not in out])
    return out


def needs_sdl(text: str) -> bool:
    return (
        "sdl2w/" in text
        or "sdl2w::" in text
        or "TRANSLATE" in text
        or "Logger" in text
        or "LOG(" in text
        or "LOG_ENDL" in text
    )


def needs_bmin(text: str) -> bool:
    return (
        "bmin/" in text
        or "bmin::" in text
        or "DynArray" in text
        or "UniquePtr" in text
        or "bmin::String" in text
        or "bmin::Map" in text
        or "bmin::List" in text
    )


def main() -> int:
    headers, sources = collect()
    by_res = {h.resolve(): h for h in headers}
    nodes = list(by_res.keys())

    raws: dict[Path, str] = {}
    bodies: dict[Path, str] = {}
    owner: dict[str, Path] = {}
    for h in headers:
        hr = h.resolve()
        raws[hr] = read(h)
        bodies[hr] = strip_body(raws[hr])
        for t in defined_types(bodies[hr]):
            owner.setdefault(t, hr)

    # edges: include dependencies only (header include graph is a DAG)
    edges: dict[Path, set[Path]] = {n: set() for n in nodes}
    for h in headers:
        hr = h.resolve()
        for dep in includes(h):
            if dep in by_res:
                edges[hr].add(dep)

    # Also import modules for forward-declared types when that does not cycle.
    # Build provisional import edges = include edges + safe fwd imports.
    import_edges: dict[Path, set[Path]] = {n: set(ds) for n, ds in edges.items()}
    for h in headers:
        hr = h.resolve()
        for name in FWD_ANY_RE.findall(bodies[hr]):
            oh = owner.get(name)
            if not oh or oh == hr or oh not in by_res:
                continue
            # Skip if oh already reaches hr (would create a cycle)
            # BFS from oh along import_edges
            seen = set()
            stack = [oh]
            reaches = False
            while stack:
                cur = stack.pop()
                if cur == hr:
                    reaches = True
                    break
                if cur in seen:
                    continue
                seen.add(cur)
                stack.extend(import_edges.get(cur, ()))
            if not reaches:
                import_edges[hr].add(oh)

    comps = sccs(nodes, import_edges)
    print(
        f"headers={len(headers)} SCCs={len(comps)} "
        f"max_scc={max(len(c) for c in comps)} multi={sum(1 for c in comps if len(c) > 1)}"
    )

    # Map header -> scc index
    comp_of = {}
    for i, c in enumerate(comps):
        for n in c:
            comp_of[n] = i

    # Unit deps between SCCs (use full import graph)
    unit_deps: dict[int, set[int]] = {i: set() for i in range(len(comps))}
    for a, ds in import_edges.items():
        for b in ds:
            ia, ib = comp_of[a], comp_of[b]
            if ia != ib:
                unit_deps[ia].add(ib)

    unit_order = topo_units(list(range(len(comps))), unit_deps)

    MODULES.mkdir(parents=True, exist_ok=True)
    (MODULES / "make").mkdir(parents=True, exist_ok=True)
    for stale in MODULES.glob("carcer*.cppm"):
        stale.unlink()

    # Assign module names
    unit_mod: dict[int, str] = {}
    header_mod: dict[Path, str] = {}
    # Include-only edges (ignore fwd-decl edges) — usually acyclic inside an SCC
    include_edges: dict[Path, set[Path]] = {n: set() for n in nodes}
    for h in headers:
        hr = h.resolve()
        for dep in includes(h):
            if dep in by_res:
                include_edges[hr].add(dep)

    unit_headers: dict[int, list[Path]] = {}
    for i in unit_order:
        members = topo_within(
            [by_res[n] for n in comps[i]],
            {
                by_res[k]: {by_res[x] for x in vs if x in by_res}
                for k, vs in include_edges.items()
            },
        )
        unit_headers[i] = members
        if len(members) == 1:
            name = mod_name_for_rel(members[0].relative_to(SRC).as_posix())
        else:
            # stable name from first member path
            base = members[0].relative_to(SRC).as_posix().rsplit(".", 1)[0].replace("/", ".")
            name = f"carcer.merge.{base}"
            print(f"  merge {name} size={len(members)}")
        unit_mod[i] = name
        for h in members:
            header_mod[h.resolve()] = name

    # Types owned by each module
    mod_owns: dict[str, set[str]] = defaultdict(set)
    for t, oh in owner.items():
        mod_owns[header_mod[oh]].add(t)

    # Emit modules
    for i in unit_order:
        name = unit_mod[i]
        members = unit_headers[i]
        member_res = {h.resolve() for h in members}
        raw_all = "\n".join(raws[h.resolve()] for h in members)
        parts = []
        non_exp_all = []
        for h in members:
            hr = h.resolve()
            rel = h.relative_to(SRC).as_posix()
            foreign = {t for t, oh in owner.items() if oh not in member_res}
            non_exp, body = split_fwds(bodies[hr], foreign)
            if non_exp.strip():
                non_exp_all.append(non_exp.rstrip())
            parts.append(f"// --- from {rel} ---\n{body.rstrip()}\n")

        # deps: other modules this unit depends on (includes + safe fwd imports)
        dep_mods = set()
        for h in members:
            for d in import_edges[h.resolve()]:
                if d not in member_res:
                    dep_mods.add(header_mod[d])
        dep_mods = sorted(dep_mods)

        body_text = "\n".join(parts)
        lines = [gmf_for(raw_all, body_text), "", f"export module {name};"]
        if needs_bmin(raw_all) or needs_bmin(body_text):
            lines.append("export import bmin.containers;")
            lines.append("import bmin.string_interop;")
        for dm in dep_mods:
            lines.append(f"export import {dm};")
        if needs_sdl(raw_all) or needs_sdl(body_text):
            lines.append("import sdl2w;")
            lines.append('#include "macros.h"')
        # Prefer importing foreign modules over local incomplete fwd decls when possible.
        # Remaining non_exp are for true incomplete deps not in dep_mods (shouldn't happen often).
        if non_exp_all:
            # Drop fwd decls whose owners we already import
            filtered = []
            for block in non_exp_all:
                keep = True
                for t in FWD_ANY_RE.findall(block):
                    oh = owner.get(t)
                    if oh and header_mod.get(oh) in dep_mods:
                        keep = False
                        break
                if keep:
                    filtered.append(block)
            if filtered:
                lines += ["", "// non-exported forward decls", "\n".join(filtered)]

        lines += ["", "export {", "", body_text, "} // export", ""]
        write(MODULES / f"{name}.cppm", "\n".join(lines))

    # umbrella
    um = ["export module carcer;", ""]
    seen = set()
    for i in unit_order:
        n = unit_mod[i]
        if n not in seen:
            um.append(f"export import {n};")
            seen.add(n)
    um.append("")
    write(MODULES / "carcer.cppm", "\n".join(um))

    # build-bmi.mk
    mk = [
        "CXX ?= g++",
        "CARCER_MOD ?= modules",
        "SDL2W_MOD ?= lib/sdl2w/modules",
        "BMIN_MOD ?= lib/sdl2w/modules/bmin",
        "FLAGS = -Wall -std=c++23 -g -fmodules-ts -I$(CARCER_MOD) -I$(SDL2W_MOD) -I$(BMIN_MOD)",
        "OBJDIR = .carcer-bmi",
        "",
        ".PHONY: all clean",
        "",
        "all: $(OBJDIR)/carcer.o",
        "\t@mkdir -p gcm.cache",
        "\t@touch gcm.cache/.carcer-ready",
        "",
        "$(OBJDIR):",
        "\t@mkdir -p $@",
        "",
    ]
    objs = []
    unit_obj = {}
    for i in unit_order:
        name = unit_mod[i]
        obj = f"$(OBJDIR)/{name}.o"
        unit_obj[i] = obj
        dep_objs = " ".join(
            unit_obj[j] for j in sorted(unit_deps[i]) if j in unit_obj
        )
        # unit_obj only filled for earlier units in topo order — deps should already be filled
        dep_objs = " ".join(unit_obj[j] for j in unit_order if j in unit_deps[i] and j in unit_obj)
        mk += [
            f"{obj}: $(CARCER_MOD)/{name}.cppm {dep_objs} | $(OBJDIR)",
            f"\t$(CXX) $(FLAGS) -c $(CARCER_MOD)/{name}.cppm -o $@",
            "",
        ]
        objs.append(obj)
    mk += [
        f"$(OBJDIR)/carcer.o: $(CARCER_MOD)/carcer.cppm {' '.join(objs)} | $(OBJDIR)",
        "\t$(CXX) $(FLAGS) -c $(CARCER_MOD)/carcer.cppm -o $@",
        "",
        "clean:",
        "\trm -rf $(OBJDIR)",
        "",
    ]
    write(MODULES / "make" / "build-bmi.mk", "\n".join(mk))
    write(
        MODULES / "bmi_objs.list",
        "\n".join([o.replace("$(OBJDIR)/", ".carcer-bmi/") for o in objs] + [".carcer-bmi/carcer.o", ""]),
    )
    write(MODULES / "module_order.txt", "\n".join(unit_mod[i] for i in unit_order) + "\n")

    # convert cpp → implementation units
    for s in sources:
        mod = None
        for ext in (".h", ".hpp"):
            cand = Path(str(s.with_suffix("")) + ext)
            if cand.resolve() in header_mod:
                mod = header_mod[cand.resolve()]
                break
        if mod is None:
            for m in INCLUDE_RE.finditer(read(s)):
                if m.group(1) != '"':
                    continue
                r = resolve(m.group(2), s)
                if r and r in header_mod:
                    mod = header_mod[r]
                    break
        if mod is None:
            mod = "carcer"
            print(f"WARNING: no module for {s.relative_to(SRC).as_posix()}, using carcer")
        body = strip_body(read(s))
        rel = s.relative_to(SRC).as_posix()
        if rel == "ui/KeyboardHeldScroll.cpp":
            body = body.replace("ScrollDirection", "HeldScrollDirection")
        if rel == "layers/ui/LayerSpecialEvent.cpp":
            body = body.replace("ui::ScrollDirection::", "ui::HeldScrollDirection::")
        body = re.sub(
            r"if \(!particle\.animation\).*?particle\.animation->update\(deltaTimeMs\);",
            "/* animation via MapView */",
            body,
            flags=re.S,
        )
        body = body.replace(
            "if (particle.animation) {\n      draw.drawAnimation(*particle.animation,",
            "auto anim = upsertAnimation(particle.animationName);\n    if (anim) {\n      draw.drawAnimation(*anim,",
        )
        write(
            s,
            "\n".join(
                [
                    gmf_for(body),
                    "",
                    f"module {mod};",
                    "import carcer;",
                    "import bmin.string_interop;",
                    "import sdl2w;",
                    '#include "macros.h"',
                    "",
                    body,
                ]
            ),
        )

    for h in headers:
        h.unlink()
    print(f"wrote {len(unit_order)} modules from {len(headers)} headers")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
