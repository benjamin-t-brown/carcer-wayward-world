#!/usr/bin/env bash
# Run acceptance tests via UCRT64 make/cxx (no Node required).
# The UI section is interactive — agents should use ./scripts/compile-ui-tests.sh
# for UI verification instead of running this script end-to-end.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT/src"

echo "Building object_files..."
make object_files -j8

CXX=$(make print_cxx)
ARGS=$(make compiler_args)

run_cpp() {
  local cpp="$1"
  echo ""
  echo "========== $cpp =========="
  $CXX "$cpp" $ARGS -o TestUi
  ./TestUi
}

# runner
for t in TestJson TestBminContainers TestConditionalEvaluator TestStringEvaluator \
         TestSpecialEventRunner TestSpecialEventIntegration; do
  run_cpp "__test__/runner/${t}.cpp"
done

# db loaders
for t in TestLoadItemTemplates TestLoadMapTemplates TestLoadAbilityTemplates \
         TestLoadCharacterTemplates TestLoadStatusEffectTemplates TestLoadSpecialEvents; do
  run_cpp "__test__/db/loaders/${t}.cpp"
done

# model
for t in TestCharacterEquip TestCharacterGive; do
  run_cpp "__test__/model/${t}.cpp"
done

# ui — folder/name pairs from test-runners/ui/*.sh
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
components	TestTouchMovePad
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
echo "All acceptance tests passed."
