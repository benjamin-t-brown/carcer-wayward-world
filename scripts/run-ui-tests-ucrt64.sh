#!/usr/bin/env bash
# Run all UI tests via UCRT64 (no Node required).
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT/src"

echo "Building object_files..."
make object_files -j8

CXX=$(make print_cxx)
ARGS=$(make compiler_args)
FAIL=0
PASS=0

run_cpp() {
  local cpp="$1"
  echo ""
  echo "========== $cpp =========="
  if $CXX "$cpp" $ARGS -o TestUi && ./TestUi; then
    PASS=$((PASS + 1))
  else
    echo "FAILED: $cpp"
    FAIL=$((FAIL + 1))
  fi
}

while IFS=$'\t' read -r folder name; do
  run_cpp "__test__/ui/${folder}/${name}.cpp"
done <<'TESTS'
components	TestConfirmModal
layouts	TestInGameLayout
pages	TestPageInventory
layers	TestLayerPickUp
layers	TestLayerInventory
elements	TestHorizontalSlider
elements	TestVerticalList
elements	TestQuad
elements	TestButtonGroup
components	TestFloatingNotificationSection
system	TestSystemFontScale
elements	TestTextBanner
elements	TestOutsetRectangle
components	TestBorderModalStandard
components	TestBorderModalSmall
components	TestBorderInGameWide
components	TestBorderInGameNarrow
components	TestChCompactInfo
components	TestInGameTitleBar
components	TestPartyMemberSwitcher
components/lists	TestListPickUp
components/lists	TestListInventory
components/lists	TestListChCompactInfoHorizontal
components/lists	TestListChCompactInfoVertical
elements	TestButtonModal
elements	TestButtonTextWrap
elements	TestButtonWorldAction
elements	TestSection
elements	TestSectionScrollable
elements	TestTextParagraph
layouts	TestModalSmall
layouts	TestModalStandard
minipages	TestMinipagePickUp
minipages	TestMinipageEvent
minipages	TestMinipageCharacterSheet
pages	TestPageCharacter
pages	TestPageTalkChoice
pages	TestPageMagicSetup
popups	TestPopupPickupItem
popups	TestPopupInventoryItem
TESTS

echo ""
echo "UI tests: $PASS passed, $FAIL failed"
[ "$FAIL" -eq 0 ]
