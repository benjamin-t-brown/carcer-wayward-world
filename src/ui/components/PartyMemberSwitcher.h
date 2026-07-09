#pragma once

#include "../UiElement.h"
#include "ui/SdlPixels.h" // IWYU pragma: keep
#include "bmin/String.h"

namespace ui {

struct PartyMemberSwitcherProps {
  bmin::String spriteName;
  int partyMemberIndex = 0;

  SDL_Color spriteBgColor = Colors::OffWhite;
  SDL_Color spriteBorderColor1 = Colors::LightGrey;
  SDL_Color spriteBorderColor2 = Colors::White;
  int spriteBorderSize = 1;
  int spriteBoxSize = 36;

  int buttonSize = 36;
  int buttonSpacing = 2;
};

// PartyMemberSwitcher - prev/next buttons flanking a party member sprite.
class PartyMemberSwitcher : public UiElement {
private:
  PartyMemberSwitcherProps props;

public:
  PartyMemberSwitcher(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~PartyMemberSwitcher() override = default;

  void setProps(const PartyMemberSwitcherProps& _props);
  PartyMemberSwitcherProps& getProps();
  const PartyMemberSwitcherProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
