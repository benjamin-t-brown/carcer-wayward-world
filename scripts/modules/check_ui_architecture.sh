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

root_importers="$(
  rg -l '^import carcer;' "${ROOT}/src" \
    --glob '*.cpp' --glob '*.cppm' 2>/dev/null || true
)"
allowed_root_importers="$(
  printf '%s\n' \
    "${ROOT}/src/__test__/modules/ImportCarcer.cpp" \
    "${ROOT}/src/main.cpp"
)"
unexpected_root_importers="$(
  comm -23 \
    <(printf '%s\n' "${root_importers}" | sed '/^$/d' | sort) \
    <(printf '%s\n' "${allowed_root_importers}" | sort)
)"

internal_widget_importers="$(
  rg -n '^import carcer\.ui\.widgets\.(foundation|views|composites);' \
    "${ROOT}/src" --glob '*.cpp' --glob '*.cppm' 2>/dev/null || true
)"

carcer_reexports="$(
  rg -n '^export import ' "${ROOT}/src/modules/_carcer.cppm" 2>/dev/null || true
)"

widget_facade_actual="$(
  rg '^export import ' "${ROOT}/src/ui/_widgets.cppm" || true
)"
widget_facade_expected="$(
  printf '%s\n' \
    'export import carcer.ui.widgets.foundation;' \
    'export import carcer.ui.widgets.views;' \
    'export import carcer.ui.widgets.composites;'
)"

main_project_imports="$(
  rg '^import (carcer|sdl2w|bmin)(\.|;)' "${ROOT}/src/main.cpp" || true
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
if [[ -n "${unexpected_root_importers}" ]]; then
  echo "Only main.cpp and the dedicated probe may import the application root:" >&2
  printf '%s\n' "${unexpected_root_importers}" >&2
  failed=1
fi
if [[ -n "${internal_widget_importers}" ]]; then
  echo "Consumers must import the public carcer.ui.widgets facade:" >&2
  printf '%s\n' "${internal_widget_importers}" >&2
  failed=1
fi
if [[ -n "${carcer_reexports}" ]]; then
  echo "The narrow carcer application root must not re-export subsystems:" >&2
  printf '%s\n' "${carcer_reexports}" >&2
  failed=1
fi
if [[ "${widget_facade_actual}" != "${widget_facade_expected}" ]]; then
  echo "carcer.ui.widgets must expose exactly its three compiler-sized build units" >&2
  diff -u <(printf '%s\n' "${widget_facade_expected}") \
    <(printf '%s\n' "${widget_facade_actual}") >&2 || true
  failed=1
fi
if [[ "${main_project_imports}" != 'import carcer;' ]]; then
  echo "main.cpp must import only the narrow carcer project API" >&2
  printf '%s\n' "${main_project_imports}" >&2
  failed=1
fi
if ! rg -q '^import carcer\.ui\.screens;' "${ROOT}/src/ui/_layers.cppm"; then
  echo "carcer.ui.layers must import carcer.ui.screens" >&2
  failed=1
fi
if ! rg -q '^import carcer\.ui\.layers;' "${ROOT}/src/modules/_carcer.cppm"; then
  echo "carcer must import carcer.ui.layers as its top UI boundary" >&2
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
