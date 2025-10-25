#pragma once

#include "../UiElement.h"

namespace ui {

// OutsetRectangle-specific properties
struct OutsetRectangleProps {
  SDL_Color color = Colors::BorderModalStandard;
  SDL_Color colorTopRight = Colors::BorderModalStandardLight;
  SDL_Color colorBottomLeft = Colors::BorderModalStandardDark;
  int borderSize = 4;
};

// OutsetRectangle element - renders a rectangle with outset border effect
// Uses Position, Size, Scale from BaseStyle
class OutsetRectangle : public UiElement {
private:
  OutsetRectangleProps props;

public:
  OutsetRectangle(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~OutsetRectangle() override = default;

  // Setters and getters for OutsetRectangle-specific properties
  void setProps(const OutsetRectangleProps& _props);
  OutsetRectangleProps& getProps();
  const OutsetRectangleProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render() override;
};

} // namespace ui
