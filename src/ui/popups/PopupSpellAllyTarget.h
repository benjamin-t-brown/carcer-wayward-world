#pragma once

#include "bmin/DynArray.h"
#include "bmin/String.h"
#include "ui/UiElement.h"

namespace ui {

struct PopupSpellAllyTargetAlly {
  bmin::String id;
  bmin::String label;
  bmin::String iconSprite;
};

struct PopupSpellAllyTargetProps {
  bmin::String casterId;
  bmin::String spellId;
  bmin::String titleText;
  bmin::String statusText;
  bmin::DynArray<PopupSpellAllyTargetAlly> allies;
};

class PopupSpellAllyTarget : public UiElement {
  PopupSpellAllyTargetProps props;

public:
  PopupSpellAllyTarget(sdl2w::Window* _window, UiElement* _parent = nullptr);

  void setProps(const PopupSpellAllyTargetProps& _props);
  PopupSpellAllyTargetProps& getProps();
  const PopupSpellAllyTargetProps& getProps() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
