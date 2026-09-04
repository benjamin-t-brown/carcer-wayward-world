#!/usr/bin/env python3
"""Move inline method bodies out of actions/*.cppm into sibling .cpp files.

Action classes are header-only and import each other, so folding combat/world/ui
into three modules would cycle at the BMI layer. Out-of-line act() (and other
methods) lets each interface import only what declarations need.
"""
from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
ACTIONS = ROOT / "src" / "state" / "actions"
EXPORT_MOD_RE = re.compile(r"^export module ([\w.]+);", re.M)
EXPORT_BRACE_RE = re.compile(r"^export \{", re.M)
CLASS_RE = re.compile(r"\b(?:class|struct)\s+(\w+)\b")
IDENT_RE = re.compile(r"[A-Za-z_]\w*")

CROSS_DROP = {
    "combat": re.compile(
        r"^(?:export\s+)?import\s+carcer\.state\.actions\.(?:world|ui|general)\b"
    ),
    "world": re.compile(
        r"^(?:export\s+)?import\s+carcer\.state\.actions\.(?:ui|general)\b"
        r"|^(?:export\s+)?import\s+carcer\.state\.actions\.combat\.(?!ActionBase\b)"
    ),
    "ui": re.compile(
        r"^(?:export\s+)?import\s+carcer\.state\.actions\.(?:world|combat|general)\b"
    ),
    "general": re.compile(
        r"^(?:export\s+)?import\s+carcer\.state\.actions\.(?:world|combat|ui)\b"
    ),
}


def bucket_of(path: Path) -> str:
    rel = path.relative_to(ACTIONS).as_posix()
    return rel.split("/", 1)[0]


def skip_string_comment(src: str, i: int) -> int:
    n = len(src)
    if src.startswith("//", i):
        j = src.find("\n", i)
        return n if j < 0 else j + 1
    if src.startswith("/*", i):
        j = src.find("*/", i + 2)
        return n if j < 0 else j + 2
    if src[i] in "'\"":
        q = src[i]
        i += 1
        while i < n:
            if src[i] == "\\":
                i += 2
                continue
            if src[i] == q:
                return i + 1
            i += 1
        return n
    return i


def match_brace(src: str, open_idx: int) -> int:
    depth = 0
    i = open_idx
    n = len(src)
    while i < n:
        nxt = skip_string_comment(src, i)
        if nxt != i:
            i = nxt
            continue
        c = src[i]
        if c == "{":
            depth += 1
        elif c == "}":
            depth -= 1
            if depth == 0:
                return i
        i += 1
    raise ValueError("unbalanced brace")


def prev_non_ws(src: str, i: int) -> int:
    i -= 1
    while i >= 0 and src[i] in " \t\r\n":
        i -= 1
    return i


def last_ident(src: str, end: int) -> str | None:
    m = None
    for m in IDENT_RE.finditer(src[max(0, end - 80) : end + 1]):
        pass
    return m.group(0) if m else None


def has_ctor_init_list(src: str, brace: int) -> bool:
    i = prev_non_ws(src, brace)
    depth = 0
    while i >= 0:
        nxt = i  # reverse scan; skip strings roughly
        if src[i] == ")":
            depth += 1
        elif src[i] == "(":
            depth -= 1
        elif depth == 0 and src[i] == ":":
            colon_colon = (i > 0 and src[i - 1] == ":") or (
                i + 1 < len(src) and src[i + 1] == ":"
            )
            if not colon_colon:
                return True
        elif depth == 0 and src[i] in ";{}":
            return False
        i -= 1
    return False


def is_function_brace(src: str, brace: int) -> str | None:
    """Return method name if `{` starts a function body, else None."""
    if has_ctor_init_list(src, brace):
        return None
    i = prev_non_ws(src, brace)
    while i >= 0:
        start = i
        while start >= 0 and src[start].isalpha():
            start -= 1
        word = src[start + 1 : i + 1]
        if word in ("override", "final", "const", "noexcept"):
            i = prev_non_ws(src, start + 1)
            continue
        break
    if i < 0 or src[i] != ")":
        return None
    depth = 0
    k = i
    while k >= 0:
        if src[k] == ")":
            depth += 1
        elif src[k] == "(":
            depth -= 1
            if depth == 0:
                break
        k -= 1
    else:
        return None
    name_end = prev_non_ws(src, k)
    return last_ident(src, name_end)


def rewrite_class_block(src: str, start: int, end: int, class_name: str) -> tuple[str, list[str]]:
    """Extract methods from one class/struct body [start,end] inclusive braces."""
    inner_start = start + 1
    inner = src[inner_start:end]
    out: list[str] = []
    defs: list[str] = []
    i = 0
    n = len(inner)
    while i < n:
        nxt = skip_string_comment(inner, i)
        if nxt != i:
            out.append(inner[i:nxt])
            i = nxt
            continue
        if inner.startswith("class ", i) or inner.startswith("struct ", i):
            cm = CLASS_RE.match(inner, i)
            if cm:
                nested = cm.group(1)
                k = cm.end()
                brace = -1
                semi = -1
                while k < n:
                    sk = skip_string_comment(inner, k)
                    if sk != k:
                        k = sk
                        continue
                    if inner[k] == ";" and brace < 0:
                        semi = k
                        break
                    if inner[k] == "{":
                        brace = k
                        break
                    k += 1
                if brace >= 0:
                    close = match_brace(inner, brace)
                    out.append(inner[i:brace])
                    new_body, ndefs = extract_in_class(inner[brace : close + 1], nested)
                    out.append(new_body)
                    defs.extend(ndefs)
                    i = close + 1
                    continue
        if inner[i] == "{":
            name = is_function_brace(inner, i)
            if name and name != class_name and not name.startswith("~"):
                close = match_brace(inner, i)
                sig_end = i  # `{`
                # include signature: from previous `;`/`{`/`}` or start of class... 
                # we already copied signature into `out` as we scanned. Need to NOT
                # have copied it. We detect `{` when we reach it, so signature is
                # still in out. Replace trailing signature's `{` with `;` and drop body.
                close_abs = close
                body = inner[i : close + 1]
                # find signature start: last ';' or '{' or '}' or access specifier in out
                sig = collect_signature(out)
                defs.append(make_out_of_line(class_name, sig, body))
                out.append(";")
                i = close + 1
                continue
        out.append(inner[i])
        i += 1
    return "{" + "".join(out) + "}", defs


def collect_signature(out: list[str]) -> str:
    text = "".join(out)
    cuts = [0]
    for tok in (";", "}", "{", "public:", "private:", "protected:"):
        idx = text.rfind(tok)
        if idx >= 0:
            cuts.append(idx + len(tok))
    return text[max(cuts) :].rstrip()


def make_out_of_line(class_name: str, sig: str, body: str) -> str:
    sig = sig.strip()
    # drop virtual, override, final from out-of-line defs
    sig = re.sub(r"\bvirtual\s+", "", sig)
    sig = re.sub(r"\s+override\b", "", sig)
    sig = re.sub(r"\s+final\b", "", sig)
    static = bool(re.match(r"static\s+", sig))
    if static:
        sig = re.sub(r"^static\s+", "", sig)
    # insert Class:: before method name (last ident before '(')
    par = sig.rfind("(")
    if par < 0:
        raise ValueError(f"no ( in signature: {sig!r}")
    head, tail = sig[:par], sig[par:]
    m = None
    for m in IDENT_RE.finditer(head):
        pass
    if not m:
        raise ValueError(f"no name in {sig!r}")
    head = head[: m.start()] + f"{class_name}::{m.group(0)}"
    return f"{head}{tail} {body}"


def extract_in_class(brace_block: str, class_name: str) -> tuple[str, list[str]]:
    """brace_block starts with '{' and ends with '}'."""
    close = match_brace(brace_block, 0)
    inner = brace_block[1:close]
    # rebuild using rewrite on a fake wrapper
    wrapped = "{" + inner + "}"
    # Use extract_in_class inner scanner only
    return rewrite_class_block(wrapped, 0, len(wrapped) - 1, class_name)


def extract_export_body(body: str) -> tuple[str, list[str]]:
    """Walk export body; for each class/struct with `{`, extract methods."""
    out: list[str] = []
    defs: list[str] = []
    i = 0
    n = len(body)
    while i < n:
        nxt = skip_string_comment(body, i)
        if nxt != i:
            out.append(body[i:nxt])
            i = nxt
            continue
        cm = CLASS_RE.match(body, i)
        if cm:
            name = cm.group(1)
            j = cm.end()
            brace = -1
            k = j
            while k < n:
                sk = skip_string_comment(body, k)
                if sk != k:
                    k = sk
                    continue
                if body[k] == ";":
                    break
                if body[k] == "{":
                    brace = k
                    break
                k += 1
            if brace >= 0:
                close = match_brace(body, brace)
                out.append(body[i:brace])
                new_block, ndefs = extract_in_class(body[brace : close + 1], name)
                out.append(new_block)
                defs.extend(ndefs)
                i = close + 1
                continue
        out.append(body[i])
        i += 1
    return "".join(out), defs


def drop_cross_imports(preamble: str, bucket: str) -> str:
    pat = CROSS_DROP[bucket]
    lines = []
    for line in preamble.splitlines(keepends=True):
        if pat.search(line.strip()):
            continue
        lines.append(line)
    return "".join(lines)


def process(path: Path, dry_run: bool) -> bool:
    text = path.read_text(encoding="utf-8")
    m = EXPORT_MOD_RE.search(text)
    if not m:
        raise SystemExit(f"no export module in {path}")
    mod = m.group(1)
    em = EXPORT_BRACE_RE.search(text)
    if not em:
        raise SystemExit(f"no export {{ in {path}")
    preamble = text[: em.end()]
    rest = text[em.end() :]
    # strip closing } // export
    end_m = re.search(r"\}\s*//\s*export\s*$", rest)
    if not end_m:
        raise SystemExit(f"no closing export in {path}")
    body, closer = rest[: end_m.start()], rest[end_m.start() :]
    new_body, defs = extract_export_body(body)
    if not defs:
        return False
    bucket = bucket_of(path)
    new_preamble = drop_cross_imports(preamble, bucket)
    new_cppm = new_preamble + new_body + closer
    if not new_cppm.endswith("\n"):
        new_cppm += "\n"

    gmf = text[: m.start()]
    gmf_includes = [
        ln.strip()
        for ln in gmf.splitlines()
        if ln.strip().startswith("#include")
    ]
    orig_pre = text[m.end() : em.start()]
    imports = [
        ln
        for ln in orig_pre.splitlines()
        if ln.strip().startswith(("import ", "export import ", "#include"))
    ]
    # implementation unit cannot import its own module
    imports = [
        ln
        for ln in imports
        if ln.strip() not in (f"import {mod};", f"export import {mod};")
    ]
    # demote export import -> import
    cpp_imps = [ln.replace("export import ", "import ", 1) if ln.strip().startswith("export import ") else ln for ln in imports]
    ns_open = "namespace state {\nnamespace actions {\n\n"
    ns_close = "\n} // namespace actions\n} // namespace state\n"
    gmf_block = (
        "module;\n" + "\n".join(gmf_includes) + "\n\n" if gmf_includes else "module;\n\n"
    )
    cpp = (
        gmf_block
        + f"module {mod};\n"
        + "\n".join(cpp_imps)
        + "\n\n"
        + ns_open
        + "\n\n".join(defs)
        + "\n"
        + ns_close
    )
    cpp_path = path.with_suffix(".cpp")
    print(f"{path.relative_to(ROOT).as_posix()}: {len(defs)} methods -> {cpp_path.name}")
    if dry_run:
        return True
    path.write_text(new_cppm, encoding="utf-8", newline="\n")
    cpp_path.write_text(cpp, encoding="utf-8", newline="\n")
    return True


def main() -> int:
    dry = "--dry-run" in sys.argv
    n = 0
    for p in sorted(ACTIONS.rglob("*.cppm")):
        if process(p, dry):
            n += 1
    print(f"extracted {n} files")
    return 0


if __name__ == "__main__":
    sys.exit(main())
