#pragma once

#include "bmin/DynArray.h"
#include "bmin/String.h"
#include "ui/UiElement.h"
#include "ui/components/lists/ListMagicSpells.h"
#include "ui/layouts/ModalStandard.h"

namespace ui {

class SectionScrollable;

struct PageMagicSetupPartyMember {
  bmin::String spriteName;
};

struct PageMagicSetupSpellEntry {
  bmin::String id;
  bmin::String label;
  bmin::String iconSprite;
  /** Shown in the label as "[cost]" before "(r)" when ready. */
  int manaCost = 0;
  /** Equipped runes meet requiredRunes — shown as "(r)" on the label. */
  bool ready = false;
  /** Required rune icons expanded by count (right-justified on the spell row). */
  bmin::DynArray<bmin::String> requiredRuneSprites;
};

struct PageMagicSetupRuneSlot {
  bool filled = false;
  bmin::String iconSprite;
};

struct PageMagicSetupElementCount {
  bmin::String iconSprite;
  int count = 0;
};

struct PageMagicSetupProps {
  int width = 0;
  int height = 0;
  bmin::String characterPlayerId;
  bmin::String characterPlayerLabel;
  bmin::String characterPlayerSprite;
  // Passed through to ModalStandard; scales headerHeight + portrait together.
  float portraitScale = 1.f;
  int partyMemberMagicIndex = 0;
  bmin::DynArray<PageMagicSetupPartyMember> partyMembers;
  bmin::DynArray<PageMagicSetupRuneSlot> runeSlots;
  int selectedRuneSlotIndex = -1;
  bmin::DynArray<PageMagicSetupElementCount> elementCounts;
  /** Single spell list (known spells); ready ones get "(r)" in the UI. */
  bmin::DynArray<PageMagicSetupSpellEntry> spells;
};

// PageMagicSetup - renders the magic page with ModalStandard layout.
class PageMagicSetup : public UiElement {
private:
  PageMagicSetupProps props;

  static constexpr int statusStripPadding = 8;
  static constexpr int manaSlotSize = 24;
  static constexpr int manaSlotGap = 4;
  // Rune sprites are 24x24 (Sprites,runes); drawSprite uses native size × scale.
  static constexpr int manaSlotIconSize = 24;
  static constexpr float manaSlotIconScale = 1.f;
  static constexpr int editRunesButtonWidth = 64;
  static constexpr int editRunesButtonHeight = 32;
  static constexpr int editRunesButtonGap = 8;
  static constexpr int elementGridCols = 4;
  static constexpr int elementCellWidth = 32;
  // Count label (~TEXT_SIZE_14) + 24px icon; was 32 and clipped the next row's numbers.
  static constexpr int elementCellHeight = 48;
  // Same 24x24 runes sprites as equipped slots.
  static constexpr int elementIconSize = 24;
  static constexpr float elementIconScale = 1.f;
  // Two available-rune rows (wide: side-by-side strip; narrow: stacked under equipped).
  static constexpr int availableRunesGridHeight = elementCellHeight * 2;
  // Matches modalLayoutFit portrait threshold (tall phone / portrait tablet).
  static constexpr float narrowAspectMin = 1.25f;
  static constexpr int spellPanelScrollBarWidth = 32;
  static constexpr int spellPanelHeaderPadding = 4;

  enum class StatusAlign { Left, Center, Right };

  bool isNarrowLayout() const;
  int equippedRowHeight() const;
  int statusAreaHeight(bool narrow) const;

  void addPortraitBackground(ModalStandard* modal);
  void addPartyMemberSelector(ModalStandard* modal);
  void addRuneSlotRow(ModalStandard* modal,
                      int contentX,
                      int contentW,
                      int rowY,
                      int rowHeight,
                      StatusAlign align);
  void addElementCountGrid(ModalStandard* modal,
                           int contentX,
                           int contentW,
                           int gridTopY,
                           StatusAlign align);
  void addSpellsPanel(int x, int y, int width, int height);
  ListMagicSpellsProps makeSpellListProps(int width) const;

public:
  PageMagicSetup(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~PageMagicSetup() override = default;

  void setProps(const PageMagicSetupProps& _props);
  PageMagicSetupProps& getProps();
  const PageMagicSetupProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
