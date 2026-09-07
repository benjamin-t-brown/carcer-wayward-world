#!/usr/bin/env bash
# Enforce the permanent UI dependency direction during the finalization pass.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
ALLOW_LEGACY_FACADE=0
if [[ "${1:-}" == "--allow-legacy-layer-facade" ]]; then
  ALLOW_LEGACY_FACADE=1
elif [[ -n "${1:-}" ]]; then
  echo "usage: $0 [--allow-legacy-layer-facade]" >&2
  exit 2
fi

lower_to_ui="$(
  rg -n '^(export )?import carcer\.ui\.' \
    "${ROOT}/src/actions" "${ROOT}/src/data" "${ROOT}/src/db" \
    "${ROOT}/src/game" "${ROOT}/src/in3" "${ROOT}/src/model" \
    "${ROOT}/src/state" 2>/dev/null || true
)"

screen_to_layers="$(
  rg -n '^(export )?import carcer\.ui\.(screens\.)?layers;' \
    "${ROOT}/src/ui/_screens.cppm" "${ROOT}/src/ui/screens" 2>/dev/null || true
)"
if [[ "${ALLOW_LEGACY_FACADE}" == "1" && -n "${screen_to_layers}" ]]; then
  screen_to_layers="$(
    printf '%s\n' "${screen_to_layers}" |
      awk '!(/_screens\.cppm:8:export import carcer\.ui\.screens\.layers;/)'
  )"
fi

failed=0
if [[ -n "${lower_to_ui}" ]]; then
  echo "Forbidden dependency from state/actions/rules/data to UI:" >&2
  printf '%s\n' "${lower_to_ui}" >&2
  failed=1
fi
if [[ -n "${screen_to_layers}" ]]; then
  echo "Forbidden dependency from screens to layers:" >&2
  printf '%s\n' "${screen_to_layers}" >&2
  failed=1
fi
if [[ "${failed}" == "1" ]]; then
  exit 1
fi

if [[ "${ALLOW_LEGACY_FACADE}" == "1" ]]; then
  echo "UI dependency direction passes with the Phase 0 legacy-facade exception"
else
  echo "UI dependency direction passes"
fi
