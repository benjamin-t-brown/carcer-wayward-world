#pragma once

#include "../UiElement.h"

namespace ui {

// Direction enum for scroll buttons
enum class ScrollDirection {
  UP,
  DOWN
};

// ButtonScroll-specific properties
struct ButtonScrollProps {
  ScrollDirection direction = ScrollDirection::UP;
  bool isSelected = false;
};

// ButtonScroll element - renders a square clickable button used to scroll windows up/down
// Uses Position, Size, Scale from BaseStyle
class ButtonScroll : public UiElement {
private:
  ButtonScrollProps props;
  bool isInHoverMode = false;
  bool isInActiveMode = false;

public:
  bool isActive = false;
  ButtonScroll(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ButtonScroll() override = default;

  // Setters and getters for button-specific properties
  void setProps(const ButtonScrollProps& _props);
  ButtonScrollProps& getProps();
  const ButtonScrollProps& getProps() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
