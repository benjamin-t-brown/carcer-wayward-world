#pragma once

#include "../UiElement.h"

namespace ui {

// Close type enum for close buttons
enum class CloseType {
  MODAL,
  POPUP
};

// ButtonClose-specific properties
struct ButtonCloseProps {
  CloseType closeType = CloseType::MODAL;
};

// ButtonClose element - renders a clickable button typically used to close a modal or popup window
// Uses Position, Size, Scale from BaseStyle
class ButtonClose : public UiElement {
private:
  ButtonCloseProps props;
  bool isInHoverMode = false;
  bool isInActiveMode = false;

public:
  bool isActive = false;
  ButtonClose(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ButtonClose() override = default;

  // Setters and getters for button-specific properties
  void setProps(const ButtonCloseProps& _props);
  ButtonCloseProps& getProps();
  const ButtonCloseProps& getProps() const;

  void build() override;
  void render() override;
};

} // namespace ui
