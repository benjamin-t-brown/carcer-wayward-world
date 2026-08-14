#pragma once

#include "bmin/DynArray.h"
#include "bmin/String.h"
#include "sdl2w/L10n.h"
#include "ui/UiElement.h"

namespace ui {

struct MinipageSpellCastSpell {
  bmin::String id;
  bmin::String label;
  bmin::String iconSprite;
  bmin::DynArray<bmin::String> requiredRuneSprites;
};

struct MinipageSpellCastProps {
  bmin::String casterId;
  int width = 0;
  int height = 0;
  bmin::String titleText = TRANSLATE("Cast Spell");
  bmin::String statusText;
  bmin::DynArray<MinipageSpellCastSpell> spells;
  bmin::String doneButtonRemoveLayerId;
};

/** ModalSmall combat cast list: known spells; row click validates then aims. */
class MinipageSpellCast : public UiElement {
private:
  MinipageSpellCastProps props;

public:
  MinipageSpellCast(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~MinipageSpellCast() override = default;

  void setProps(const MinipageSpellCastProps& _props);
  MinipageSpellCastProps& getProps();
  const MinipageSpellCastProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
