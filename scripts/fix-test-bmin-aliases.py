#!/usr/bin/env python3
"""Migrate bare bmin type aliases in src/__test__ to bmin:: qualified names."""
import os
import re
import sys

ROOT = os.path.join(os.path.dirname(__file__), "..", "src", "__test__")

BMIN_INCLUDES = {
    "DynArray": '#include "bmin/DynArray.h"',
    "UniquePtr": '#include "bmin/UniquePtr.h"',
    "String": '#include "bmin/String.h"',
    "List": '#include "bmin/List.h"',
    "Map": '#include "bmin/Map.h"',
    "StringInterop": '#include "bmin/StringInterop.h"',
}


def needs_migration(content: str) -> bool:
    patterns = [
        r"(?<!bmin::)(?<!std::)\bDynArray<",
        r"(?<!bmin::)(?<!std::)\bUniquePtr<",
        r"(?<!bmin::)(?<!std::)\bmakeUnique<",
        r"(?<!bmin::)(?<!std::)\bList<",
        r"(?<!bmin::)(?<!std::)Map<String",
        r"(?<!bmin::)(?<!std::)\bString\b",
    ]
    return any(re.search(p, content) for p in patterns)


def fix_content(content: str) -> str:
    content = content.replace("bmin::bmin::", "bmin::")
    lines = content.splitlines(keepends=True)
    new_lines = []
    for line in lines:
        if line.strip().startswith("#include"):
            new_lines.append(line)
            continue
        nl = line
        nl = re.sub(r"(?<!bmin::)DynArray<", "bmin::DynArray<", nl)
        nl = re.sub(r"(?<!bmin::)UniquePtr<", "bmin::UniquePtr<", nl)
        nl = re.sub(r"(?<!bmin::)makeUnique<", "bmin::makeUnique<", nl)
        nl = re.sub(r"(?<!bmin::)List<", "bmin::List<", nl)
        nl = re.sub(r"\bMap<String", "Map<bmin::String", nl)
        nl = re.sub(r"(?<!bmin::)(?<!std::)String\b", "bmin::String", nl)
        new_lines.append(nl)
    content = "".join(new_lines)
    for bad, good in [
        ("bmin::StringUtil", "StringUtil"),
        ("bmin::StringEvaluator", "StringEvaluator"),
        ("bmin::StringStream", "StringStream"),
        ("bmin::StringView", "StringView"),
        ("bmin::StringInterop", "StringInterop"),
        ("bmin::tobmin::String", "bmin::toString"),
    ]:
        content = content.replace(bad, good)
    return content


def add_includes(content: str) -> str:
    needs = set()
    if "bmin::DynArray" in content:
        needs.add("DynArray")
    if "bmin::UniquePtr" in content or "bmin::makeUnique" in content:
        needs.add("UniquePtr")
    if "bmin::String" in content:
        needs.add("String")
    if "bmin::List" in content:
        needs.add("List")
    if "bmin::Map" in content:
        needs.add("Map")
    if "bmin::toStringView" in content:
        needs.add("StringInterop")

    inc_lines = []
    for key in ["String", "DynArray", "UniquePtr", "List", "Map", "StringInterop"]:
        if key in needs:
            inc = BMIN_INCLUDES[key]
            if inc not in content:
                inc_lines.append(inc + "\n")

    if not inc_lines:
        return content

    m = re.search(r"((?:#include[^\n]+\n)+)", content)
    if m:
        insert_at = m.end()
        return content[:insert_at] + "".join(inc_lines) + content[insert_at:]
    return "".join(inc_lines) + content


def fix_file(path: str) -> bool:
    with open(path, encoding="utf-8") as f:
        content = f.read()
    if not needs_migration(content):
        return False
    updated = fix_content(content)
    updated = add_includes(updated)
    if updated == content:
        return False
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        f.write(updated)
    return True


def main() -> int:
    fixed = []
    for dirpath, _, files in os.walk(ROOT):
        for fn in files:
            if fn.endswith(".cpp"):
                p = os.path.join(dirpath, fn)
                if fix_file(p):
                    fixed.append(p)
    print(f"Fixed {len(fixed)} files")
    for p in sorted(fixed):
        print(os.path.relpath(p, os.path.join(os.path.dirname(__file__), "..")))
    return 0


if __name__ == "__main__":
    sys.exit(main())
