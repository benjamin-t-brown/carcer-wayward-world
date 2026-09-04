#!/usr/bin/env python3
"""Move out-of-line method bodies after `} // export` into the class in export { }.

  python scripts/modules/inline_export_methods.py src/actions
"""
from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
EXPORT_END = "} // export"


def matching(text: str, open_idx: int, opener: str, closer: str) -> int:
    depth = 0
    i = open_idx
    while i < len(text):
        ch = text[i]
        if ch == opener:
            depth += 1
        elif ch == closer:
            depth -= 1
            if depth == 0:
                return i
        i += 1
    raise ValueError("unbalanced " + opener)


def parse_out_of_line(trailing: str) -> list[tuple[str, str, str, str]]:
    """Return (class, name, decl_prefix_and_args, body)."""
    methods: list[tuple[str, str, str, str]] = []
    sig_re = re.compile(
        r"(?:^|\n)((?:static\s+)?(?:[\w:<>,\s\*&]+?)\s+)(\w+)::(\w+)\s*\(",
    )
    pos = 0
    while True:
        m = sig_re.search(trailing, pos)
        if not m:
            break
        prefix, cls, name = m.group(1), m.group(2), m.group(3)
        paren_open = m.end() - 1
        paren_close = matching(trailing, paren_open, "(", ")")
        rest = trailing[paren_close + 1 :].lstrip()
        if not rest.startswith("{"):
            raise SystemExit(f"expected '{{' after {cls}::{name}")
        brace_open = paren_close + 1 + trailing[paren_close + 1 :].find("{")
        brace_close = matching(trailing, brace_open, "{", "}")
        args = trailing[paren_open : paren_close + 1]
        quals = trailing[paren_close + 1 : brace_open].strip()
        body = trailing[brace_open + 1 : brace_close]
        decl_head = f"{prefix.strip()} {name}{args}"
        if quals:
            decl_head += f" {quals}"
        methods.append((cls, name, decl_head, body))
        pos = brace_close + 1
    leftover = re.sub(r"\bnamespace\s+\w+\s*\{", "", trailing)
    leftover = re.sub(r"\} // namespace.*", "", leftover)
    leftover = leftover.strip()
    # Strip consumed method text for leftover check: if we parsed methods,
    # ignore leftover that is only braces / namespace close.
    leftover = re.sub(r"[\s{}]+", "", leftover)
    leftover = re.sub(r"[\w:]+::\w+", "", leftover)
    return methods


def class_span(export_body: str, cls: str) -> tuple[int, int]:
    m = re.search(rf"\bclass\s+{re.escape(cls)}\b[^{{]*\{{", export_body)
    if not m:
        raise SystemExit(f"class {cls} not found")
    open_idx = m.end() - 1
    close_idx = matching(export_body, open_idx, "{", "}")
    return open_idx + 1, close_idx


def reindent_body(body: str, indent: str) -> str:
    lines = body.splitlines()
    while lines and not lines[0].strip():
        lines.pop(0)
    while lines and not lines[-1].strip():
        lines.pop()
    nonempty = [ln for ln in lines if ln.strip()]
    if not nonempty:
        return ""
    min_i = min(len(ln) - len(ln.lstrip(" ")) for ln in nonempty)
    inner = indent + "  "
    out: list[str] = []
    for ln in lines:
        if not ln.strip():
            out.append("")
            continue
        stripped = ln[min_i:] if ln.startswith(" " * min_i) else ln.lstrip()
        out.append(inner + stripped)
    return "\n".join(out)


def replace_decl(class_body: str, name: str, decl_head: str, body: str) -> str:
    # Match a (possibly multiline) declaration of `name(...)` ending in `;`
    decl_re = re.compile(
        rf"(?P<indent>^[ \t]+)(?P<static>static\s+)?"
        rf"(?P<ret>(?:[\w:<>,\*&]|[ \t])+?)\s+{re.escape(name)}\s*"
        rf"(?P<args>\([^;]*?\))\s*(?P<quals>(?:override|const|final|[ \t])*)\s*;",
        re.M | re.S,
    )
    m = decl_re.search(class_body)
    if not m:
        raise SystemExit(f"declaration for {name} not found in class")
    indent = m.group("indent")
    static = m.group("static") or ""
    ret = re.sub(r"\s+", " ", m.group("ret").strip())
    args = re.sub(r"\s+", " ", m.group("args").strip())
    quals = " ".join(m.group("quals").split())
    header = f"{indent}{static}{ret} {name}{args}"
    if quals:
        header += f" {quals}"
    inner = reindent_body(body, indent)
    replacement = f"{header} {{\n{inner}\n{indent}}}"
    return class_body[: m.start()] + replacement + class_body[m.end() :]


def inline_file(path: Path) -> bool:
    text = path.read_text(encoding="utf-8")
    marker = "\n" + EXPORT_END
    idx = text.find(marker)
    if idx < 0:
        return False
    after = text[idx + len(marker) :].strip()
    if not after:
        return False
    if "::" not in after:
        return False

    methods = parse_out_of_line(after)
    if not methods:
        raise SystemExit(f"no out-of-line methods in {path}")

    before = text[: idx + len(marker)]
    export_start = before.rfind("\nexport {\n")
    if export_start < 0:
        raise SystemExit(f"no export {{ in {path}")
    body_start = export_start + len("\nexport {\n")
    export_body = before[body_start:idx]

    # Apply replacements last-to-first within each class so earlier offsets stay valid
    # if we replace whole class bodies one class at a time.
    by_class: dict[str, list[tuple[str, str, str]]] = {}
    for cls, name, _head, body in methods:
        by_class.setdefault(cls, []).append((name, _head, body))

    for cls, items in by_class.items():
        start, end = class_span(export_body, cls)
        class_body = export_body[start:end]
        for name, _head, body in items:
            class_body = replace_decl(class_body, name, _head, body)
        export_body = export_body[:start] + class_body + export_body[end:]

    new_text = before[:body_start] + export_body + "\n" + EXPORT_END + "\n"
    path.write_text(new_text, encoding="utf-8", newline="\n")
    return True


def main() -> int:
    dirs = sys.argv[1:] or ["src/actions"]
    n = 0
    for arg in dirs:
        folder = Path(arg)
        if not folder.is_absolute():
            folder = ROOT / folder
        for p in sorted(folder.rglob("*.cppm")):
            if inline_file(p):
                n += 1
                print("inlined", p.relative_to(ROOT).as_posix())
    print(f"updated {n} files")
    return 0


if __name__ == "__main__":
    sys.exit(main())
