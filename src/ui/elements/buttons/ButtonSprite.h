#pragma once

#include "../../UiElement.h"
#include "ui/colors.h"
#include "bmin/String.h"

namespace ui {

struct ButtonSpriteProps {
  bmin::String spriteName;
  int spriteWidth = 16;
  int spriteHeight = 16;
  int padding = 2;
  bool isSelected = false;

  SDL_Color bgColor = Colors::ButtonModalGrey1;
  SDL_Color bgColorTopRight = Colors::ButtonModalGrey2;
  SDL_Color bgColorBottomLeft = Colors::ButtonModalGrey3;
  int borderSize = 2;

  SDL_Color selectedBgColor = Colors::ButtonModalSelected;
  SDL_Color selectedBgColorTopRight = Colors::ButtonModalSelected;
  SDL_Color selectedBgColorBottomLeft = Colors::ButtonModalSelected;
  int selectedBorderSize = 2;
};

// ButtonSprite - clickable button displaying a sprite with uniform padding.
class ButtonSprite : public UiElement {
private:
  ButtonSpriteProps props;
  bool isInActiveMode = false;

  int getLogicalWidth() const;
  int getLogicalHeight() const;

public:
  bool isActive = false;
  ButtonSprite(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ButtonSprite() override = default;

  void setProps(const ButtonSpriteProps& _props);
  ButtonSpriteProps& getProps();
  const ButtonSpriteProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
