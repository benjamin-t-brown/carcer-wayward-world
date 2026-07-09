#pragma once

#include "../../UiElement.h"
#include "bmin/String.h"

namespace ui {

// ButtonModal-specific properties
struct ButtonModalProps {
  bmin::String text;
  bool isSelected = false;

  SDL_Color bgColor = Colors::ButtonModalGrey1;
  SDL_Color bgColorTopRight = Colors::ButtonModalGrey2;
  SDL_Color bgColorBottomLeft = Colors::ButtonModalGrey3;
};

// ButtonModal element - renders a clickable button typically used inside modal windows
// Uses Position, Size, Scale from BaseStyle
class ButtonModal : public UiElement {
private:
  ButtonModalProps props;
  bool isInHoverMode = false;
  bool isInActiveMode = false;

public:
  bool isActive = false;
  ButtonModal(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ButtonModal() override = default;

  // Setters and getters for button-specific properties
  void setProps(const ButtonModalProps& _props);
  ButtonModalProps& getProps();
  const ButtonModalProps& getProps() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
