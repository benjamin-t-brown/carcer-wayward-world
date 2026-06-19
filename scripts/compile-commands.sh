#!/bin/bash

# Generate compile_commands.json for clangd from src/Makefile variables.
# No external tools required beyond make and bash.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
SRC_DIR="$ROOT_DIR/src"
OUTPUT="$ROOT_DIR/compile_commands.json"

json_string() {
  local value="$1"
  value="${value//\\/\\\\}"
  value="${value//\"/\\\"}"
  printf '%s' "$value"
}

abs_directory() {
  if command -v cygpath >/dev/null 2>&1; then
    cygpath -m "$(pwd)"
  else
    pwd
  fi
}

to_json_path() {
  local path="$1"
  if command -v cygpath >/dev/null 2>&1; then
    cygpath -m "$path" 2>/dev/null || printf '%s' "$path"
  else
    printf '%s' "$path"
  fi
}

cd "$SRC_DIR"

echo "Reading compile settings from Makefile..." >&2

MAKE_DB=""
TMP_OUTPUT=""
cleanup() {
  rm -f "$MAKE_DB" "$TMP_OUTPUT"
}
trap cleanup EXIT

MAKE_DB="$(mktemp)"
make --no-print-directory -s print_compile_db > "$MAKE_DB"

CXX=""
COMPILE_FLAGS=()
SOURCES=()

while IFS= read -r line || [[ -n "$line" ]]; do
  case "$line" in
    CXX:*)
      CXX="${line#CXX:}"
      ;;
    FLAGS:*)
      read -r -a COMPILE_FLAGS <<< "${line#FLAGS:}"
      ;;
    SOURCE:*)
      SOURCES+=("${line#SOURCE:}")
      ;;
  esac
done < "$MAKE_DB"

if [[ -z "$CXX" ]]; then
  echo "Error: make print_compile_db did not return CXX" >&2
  exit 1
fi

TEST_SOURCES=()
TEST_LIST="$(mktemp)"
find __test__ -name '*.cpp' 2>/dev/null | sort > "$TEST_LIST" || true
while IFS= read -r line; do
  [[ -n "$line" ]] && TEST_SOURCES+=("$line")
done < "$TEST_LIST"
rm -f "$TEST_LIST"

DIRECTORY="$(abs_directory)"
TMP_OUTPUT="$(mktemp)"

echo "Writing compile_commands.json..." >&2

{
  printf '[\n'
  first=1

  emit_entry() {
    local source="$1"
    local object="${source%.cpp}.o"
    local file_path
    file_path="$(to_json_path "$source")"

    if [[ $first -eq 1 ]]; then
      first=0
    else
      printf ',\n'
    fi

    printf '  {\n'
    printf '    "directory": "%s",\n' "$(json_string "$DIRECTORY")"
    printf '    "arguments": [\n'
    printf '      "%s"' "$(json_string "$CXX")"
    for flag in "${COMPILE_FLAGS[@]}"; do
      printf ',\n      "%s"' "$(json_string "$flag")"
    done
    printf ',\n      "-MMD",\n'
    printf '      "-MP",\n'
    printf '      "-c",\n'
    printf '      "%s",\n' "$(json_string "$source")"
    printf '      "-o",\n'
    printf '      "%s"\n' "$(json_string "$object")"
    printf '    ],\n'
    printf '    "file": "%s"\n' "$(json_string "$file_path")"
    printf '  }'
  }

  for source in "${SOURCES[@]}"; do
    [[ -n "$source" ]] || continue
    emit_entry "$source"
  done

  for source in "${TEST_SOURCES[@]}"; do
    [[ -n "$source" ]] || continue
    emit_entry "$source"
  done

  printf '\n]\n'
} > "$TMP_OUTPUT"

mv -f "$TMP_OUTPUT" "$OUTPUT"
TMP_OUTPUT=""
trap - EXIT
rm -f "$MAKE_DB"
MAKE_DB=""

echo "Wrote $OUTPUT ($((${#SOURCES[@]} + ${#TEST_SOURCES[@]})) entries)" >&2
