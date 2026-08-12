#pragma once

#include "bmin/DynArray.h"
#include "bmin/String.h"
#include "ui/UiElement.h"

namespace ui {

struct ListMagicSpellsPropsSpell {
  bmin::String id;
  bmin::String label;
  bmin::String iconSprite;
  /** Required rune icons (already expanded by count), drawn right-justified. */
  bmin::DynArray<bmin::String> requiredRuneSprites;
};

struct ListMagicSpellsProps {
  bmin::DynArray<ListMagicSpellsPropsSpell> spells;
  int width = 0;
  int lineHeight = 32;
  int lineGap = 2;
  int paddingTop = 4;
  int paddingBottom = 12;
  // When set with a spell id, row click opens the spell info popup.
  bool enableSpellInfoOnClick = true;
};

// ListMagicSpells - vertical list of spell rows (icon + label), read-only
class ListMagicSpells : public UiElement {
private:
  ListMagicSpellsProps props;

  // Fallback only; centering uses the sprite's native size (drawSprite ignores w/h).
  static constexpr int iconSpriteSize = 24;
  static constexpr float iconScale = 1.f;
  static constexpr int labelGapAfterIcon = 12;
  // Rune sprites are 24x24; keep visual size within the 32px row.
  static constexpr int requiredRuneIconSize = 24;
  static constexpr float requiredRuneIconScale = 1.f;
  static constexpr int requiredRuneGap = 2;
  static constexpr int requiredRunesRightPadding = 4;

  UiElement* createSpellElement(const ListMagicSpellsPropsSpell& spell);

public:
  ListMagicSpells(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ListMagicSpells() override = default;

  void setProps(const ListMagicSpellsProps& props);
  const ListMagicSpellsProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
