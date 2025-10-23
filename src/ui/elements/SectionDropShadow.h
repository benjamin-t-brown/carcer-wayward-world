#pragma once

#include "../UiElement.h"
#include "ui/colors.h"

namespace ui {

// SectionDropShadow-specific properties
struct SectionDropShadowProps {
  SDL_Color backgroundColor = Colors::White; // White by default
  SDL_Color shadowColor = Colors::Black; // Black shadow
  int shadowOffsetX = -8; // Offset left
  int shadowOffsetY = 8; // Offset top
  int borderSize = 2;
  // borderColor is same as shadowColor
  bool isSelected = false;
};

// SectionDropShadow element - renders a section with white background and black drop shadow
// Uses Position, Size, Scale from BaseStyle
class SectionDropShadow : public UiElement {
private:
  SectionDropShadowProps props;

public:
  SectionDropShadow(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~SectionDropShadow() override = default;

  // Setters and getters for section-specific properties
  void setProps(const SectionDropShadowProps& _props);
  SectionDropShadowProps& getProps();
  const SectionDropShadowProps& getProps() const;

  void addChild(std::unique_ptr<UiElement> child);

  void build() override;
  void render() override;
};

} // namespace ui
