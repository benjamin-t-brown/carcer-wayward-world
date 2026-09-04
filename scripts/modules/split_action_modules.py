#!/usr/bin/env python3
"""Split carcer.actions.{ui,world,combat} barrels into one module per
action file (PlaySound shape): thin .cppm, act() stays in the sibling .cpp.

  python scripts/modules/split_action_modules.py
  python scripts/modules/gen_bmi_makefile.py
"""
from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SRC = ROOT / "src"
ACTIONS = SRC / "state" / "actions"

SECTION_RE = re.compile(r"^// --- from (.+?) ---\s*$", re.M)
EXPORT_MOD_RE = re.compile(r"^export module ([\w.]+);", re.M)
MODULE_RE = re.compile(r"^module ([\w.]+);", re.M)
IMPORT_LINE_RE = re.compile(r"^(export\s+)?import\s+([\w.]+)\s*;\s*$")
TYPE_RE = re.compile(r"\b(?:class|struct|enum class|enum)\s+(\w+)")
BARRELS = {
    "carcer.actions.ui",
    "carcer.actions.world",
    "carcer.actions.combat",
}
BARREL_FILES = {
    "ui": ACTIONS / "ui" / "ui.cppm",
    "world": ACTIONS / "world" / "world.cppm",
    "combat": ACTIONS / "combat" / "combat.cppm",
}

# Types that appear in action *declarations* → interface imports.
DECL_IMPORTS: list[tuple[re.Pattern[str], str]] = [
    (re.compile(r"\bHeldMove\b|\bUiFloatingNotificationType\b"), "carcer.state.State"),
    (re.compile(r"\bmodel::RuneType\b"), "carcer.model.templates.RuneTypes"),
    (re.compile(r"\bmodel::ItemInstance\b"), "carcer.model.instances.ItemInstance"),
    (re.compile(r"\bmodel::TileXY\b"), "carcer.model.instances.MapInstance"),
    (re.compile(r"\bmodel::CombatActionType\b|\bmodel::SpellTargetInfo\b"),
     "carcer.model.Combat"),
    (re.compile(r"\bmodel::AbilityTemplate\b"), "carcer.model.templates.AbilityTypes"),
    (re.compile(r"\bmodel::CharacterInstance\b"),
     "carcer.model.instances.CharacterInstance"),
    (re.compile(r"\bgame::ActiveMapOrchestrator\b"), "carcer.game.map"),
    (re.compile(r"\bmodel::WorldActionMode\b|\bmodel::CameraMode\b"),
     "carcer.model.instances.World"),
    (re.compile(r"\bmodel::ProjectilePath\b"), "carcer.model.templates.AbilityTypes"),
    (re.compile(r"\bmodel::TravelTrigger\b"), "carcer.model.templates.Maps"),
    (re.compile(r"\bmodel::CharacterPlayer\b"),
     "carcer.model.instances.CharacterPlayer"),
    (re.compile(r"\bmodel::Player\b"), "carcer.model.instances.Player"),
]


def export_body(text: str) -> str:
    m = re.search(r"^export \{", text, re.M)
    if not m:
        raise SystemExit("no export { in barrel")
    body = text[m.end() :]
    body = re.sub(r"\}\s*//\s*export\s*$", "", body.strip())
    return body


def split_sections(body: str) -> list[tuple[str, str]]:
    parts = SECTION_RE.split(body)
    out: list[tuple[str, str]] = []
    # parts[0] is preamble before first marker (should be empty / whitespace)
    i = 1
    while i + 1 < len(parts):
        out.append((parts[i].strip(), parts[i + 1].strip() + "\n"))
        i += 2
    return out


def module_and_path(from_path: str) -> tuple[str, Path]:
    rel = from_path.replace("\\", "/")
    if rel.endswith(".hpp") or rel.endswith(".h"):
        rel = rel.rsplit(".", 1)[0]
    if rel.endswith("ActionBase"):
        return (
            "carcer.actions.combat.CombatAction",
            ACTIONS / "combat" / "CombatAction.cppm",
        )
    p = SRC / (rel + ".cppm")
    stem = Path(rel).name
    domain = Path(rel).parts[2]  # actions/<domain>/...
    return f"carcer.actions.{domain}.{stem}", p


def gfrag_for(decl: str) -> str:
    headers = ["cstddef", "cstdint", "utility"]
    extra = [
        ("std::optional", "optional"),
        ("std::vector", "vector"),
        ("std::type_info", "typeinfo"),
        ("std::type_index", "typeindex"),
        ("std::abs", "cmath"),
        ("std::rand", "cstdlib"),
        ("std::runtime_error", "stdexcept"),
    ]
    for needle, hdr in extra:
        if needle in decl and hdr not in headers:
            headers.append(hdr)
    lines = ["module;"] + [f"#include <{h}>" for h in headers]
    return "\n".join(lines) + "\n\n"


def interface_imports(decl: str, own: str) -> list[str]:
    imports: list[str] = [
        "export import bmin.containers;",
        "import bmin.string_interop;",
    ]
    if own.endswith(".CombatAction") or re.search(
        r":\s*public\s+AbstractAction\b", decl
    ):
        imports.append("export import carcer.state.AbstractAction;")
    if own != "carcer.actions.combat.CombatAction" and re.search(
        r":\s*public\s+CombatAction\b", decl
    ):
        imports.append("export import carcer.actions.combat.CombatAction;")
    seen = {line.split()[-1].rstrip(";") for line in imports if line.startswith(("export import", "import"))}
    # parse module names from the lines we added
    def add(mod: str, export: bool = True) -> None:
        if mod in seen or mod == own:
            return
        seen.add(mod)
        prefix = "export import" if export else "import"
        imports.append(f"{prefix} {mod};")

    for pat, mod in DECL_IMPORTS:
        if pat.search(decl):
            add(mod)
    imports.append("import sdl2w;")
    imports.append('#include "macros.h"')
    return imports


def write_cppm(path: Path, name: str, decl: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    body = "\n".join(
        [
            gfrag_for(decl).rstrip(),
            f"export module {name};",
            *interface_imports(decl, name),
            "",
            "export {",
            "",
            decl.rstrip(),
            "",
            "} // export",
            "",
        ]
    )
    path.write_text(body, encoding="utf-8", newline="\n")


def retarget_cpp(cpp: Path, name: str) -> None:
    text = cpp.read_text(encoding="utf-8")
    new, n = re.subn(
        r"^module carcer\.state\.actions\.(ui|world|combat);",
        f"module {name};",
        text,
        count=1,
        flags=re.M,
    )
    if n != 1:
        raise SystemExit(f"could not retarget module line in {cpp}")
    cpp.write_text(new, encoding="utf-8", newline="\n")


def index_action_types() -> dict[str, str]:
    table: dict[str, str] = {}
    for p in ACTIONS.rglob("*.cppm"):
        text = p.read_text(encoding="utf-8")
        em = EXPORT_MOD_RE.search(text)
        if not em:
            continue
        mod = em.group(1)
        m = re.search(r"^export \{", text, re.M)
        if not m:
            continue
        body = text[m.end() :]
        for t in TYPE_RE.findall(body):
            if t in table and table[t] != mod:
                raise SystemExit(f"duplicate action type {t}: {table[t]} vs {mod}")
            table[t] = mod
    return table


def used_action_modules(text: str, table: dict[str, str], own: str | None) -> list[str]:
    found: set[str] = set()
    # Ignore comment-only mentions ("queued by PlaySound", "e.g. ModifyHP").
    code = "\n".join(
        re.sub(r"//.*", "", line)
        for line in text.splitlines()
        if not line.lstrip().startswith("//")
    )
    names = sorted(table, key=len, reverse=True)
    for name in names:
        if not re.search(rf"\b{re.escape(name)}\b", code):
            continue
        mod = table[name]
        if mod != own:
            found.add(mod)
    return sorted(found)


def own_module(text: str) -> str | None:
    em = EXPORT_MOD_RE.search(text) or MODULE_RE.search(text)
    return em.group(1) if em else None


def rewrite_imports(path: Path, table: dict[str, str]) -> bool:
    rel = path.relative_to(SRC).as_posix()
    if rel.startswith("__test__/"):
        return False
    text = path.read_text(encoding="utf-8")
    own = own_module(text)
    if own == "carcer":
        return False
    need = used_action_modules(text, table, own)
    lines = text.splitlines(keepends=True)
    out: list[str] = []
    removed = False
    existing: set[str] = set()
    insert_at: int | None = None
    for i, line in enumerate(lines):
        m = IMPORT_LINE_RE.match(line.strip())
        if m:
            exp, mod = m.group(1), m.group(2)
            if insert_at is None:
                insert_at = len(out)
            if mod in BARRELS:
                removed = True
                continue
            existing.add(mod)
        out.append(line)
        if insert_at is None and (
            EXPORT_MOD_RE.match(line.strip()) or MODULE_RE.match(line.strip())
        ):
            insert_at = len(out)
    add = [m for m in need if m not in existing]
    if not add and not removed:
        return False
    if insert_at is None:
        raise SystemExit(f"no module line in {path}")
    insert_lines = [f"import {m};\n" for m in add]
    out[insert_at:insert_at] = insert_lines
    path.write_text("".join(out), encoding="utf-8", newline="\n")
    return True


def update_umbrella(action_mods: list[str]) -> None:
    path = SRC / "modules" / "carcer.cppm"
    text = path.read_text(encoding="utf-8")
    lines = text.splitlines(keepends=True)
    out: list[str] = []
    inserted = False
    for line in lines:
        s = line.strip()
        m = IMPORT_LINE_RE.match(s)
        if m and m.group(2) in BARRELS | {"carcer.actions.general.PlaySound"}:
            if not inserted:
                for mod in action_mods:
                    out.append(f"export import {mod};\n")
                inserted = True
            continue
        out.append(line)
    if not inserted:
        raise SystemExit("umbrella had no action barrel imports to replace")
    path.write_text("".join(out), encoding="utf-8", newline="\n")


def iter_src_units() -> list[Path]:
    out: list[Path] = []
    for p in SRC.rglob("*"):
        if p.suffix not in {".cpp", ".cppm"}:
            continue
        rel = p.relative_to(SRC).as_posix()
        if rel.startswith(("lib/sdl2w/", "lib/bmin/")):
            continue
        out.append(p)
    return out


def main() -> int:
    created: list[tuple[str, Path]] = []
    for domain, barrel in BARREL_FILES.items():
        body = export_body(barrel.read_text(encoding="utf-8"))
        for from_path, decl in split_sections(body):
            name, dest = module_and_path(from_path)
            write_cppm(dest, name, decl)
            created.append((name, dest))
            cpp = dest.with_suffix(".cpp")
            if cpp.exists():
                retarget_cpp(cpp, name)
            print(f"  {name}  <- {from_path}")
        barrel.unlink()
        print(f"removed {barrel.relative_to(ROOT).as_posix()}")

    table = index_action_types()
    print(f"indexed {len(table)} action types")
    n = 0
    for p in iter_src_units():
        if rewrite_imports(p, table):
            n += 1
            print("rewrote", p.relative_to(ROOT).as_posix())
    print(f"rewrote {n} files")

    action_mods = sorted(
        {
            EXPORT_MOD_RE.search(p.read_text(encoding="utf-8")).group(1)
            for p in ACTIONS.rglob("*.cppm")
        }
    )
    update_umbrella(action_mods)
    print(f"umbrella export imports {len(action_mods)} action modules")
    return 0


if __name__ == "__main__":
    sys.exit(main())
