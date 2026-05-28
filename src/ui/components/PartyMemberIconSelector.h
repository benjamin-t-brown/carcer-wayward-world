#pragma once

#include "../UiElement.h"
#include "ui/SdlPixels.h" // IWYU pragma: keep
#include <string>
#include <vector>

namespace ui {

enum class PartyMemberIconSelectorTarget { INVENTORY, PICKUP };

struct PartyMemberIconSelectorProps {
  std::vector<std::string> members;
  int selectedIndex = 0;
  PartyMemberIconSelectorTarget target = PartyMemberIconSelectorTarget::INVENTORY;
  int iconSize = 32;
  int iconGap = 4;

  SDL_Color spriteBgColor = Colors::LightGrey;
  SDL_Color spriteBgColorTopRight = Colors::White;
  SDL_Color spriteBgColorBottomLeft = Colors::ButtonModalGrey2;

  SDL_Color spriteSelectedBgColor = Colors::ButtonModalSelected;
  SDL_Color spriteSelectedBgColorTopRight = Colors::ButtonModalSelected;
  SDL_Color spriteSelectedBgColorBottomLeft = Colors::ButtonModalSelected;
};

class PartyMemberIconSelector : public UiElement {
private:
  PartyMemberIconSelectorProps props;

public:
  PartyMemberIconSelector(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~PartyMemberIconSelector() override = default;

  void setProps(const PartyMemberIconSelectorProps& _props);
  PartyMemberIconSelectorProps& getProps();
  const PartyMemberIconSelectorProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
