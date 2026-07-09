#pragma once

#include "../../UiElement.h"
#include "ui/colors.h"

namespace ui {

struct BorderDropShadowProps {
  int width = 0;
  int height = 0;
  SDL_Color backgroundColor = Colors::White;
  SDL_Color shadowColor = Colors::Black;
  int shadowOffsetX = -8;
  int shadowOffsetY = 8;
  int borderSize = 2;
  bool isSelected = false;
};

// BorderDropShadow - panel with fill, offset drop shadow, and outline border.
// Content children are laid out in logical space inside an internal scaled Quad.
class BorderDropShadow : public UiElement {
  BorderDropShadowProps props;

public:
  BorderDropShadow(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~BorderDropShadow() override = default;

  void setProps(const BorderDropShadowProps& _props);
  BorderDropShadowProps& getProps();
  const BorderDropShadowProps& getProps() const;

  void addChild(UiElement* child) override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
