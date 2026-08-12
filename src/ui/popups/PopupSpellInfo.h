#pragma once

#include "bmin/DynArray.h"
#include "bmin/String.h"
#include "ui/UiElement.h"
#include "ui/popups/PopupInventoryItem.h"

namespace ui {

struct PopupSpellInfoRuneReq {
  bmin::String iconSprite;
  int count = 1;
};

struct PopupSpellInfoProps {
  bmin::String label;
  bmin::String description;
  bmin::String spriteName;
  int manaCost = 0;
  bmin::DynArray<PopupSpellInfoRuneReq> requiredRunes;
  PopupOrientation orientation = WIDE;
};

// Spell info popup: name, mana cost, required runes, description; close only.
class PopupSpellInfo : public UiElement {
  PopupSpellInfoProps props;

public:
  PopupSpellInfo(sdl2w::Window* _window,
                 UiElement* _parent = nullptr,
                 PopupOrientation _orientation = WIDE);

  void setProps(const PopupSpellInfoProps& _props);
  PopupSpellInfoProps& getProps();
  const PopupSpellInfoProps& getProps() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
