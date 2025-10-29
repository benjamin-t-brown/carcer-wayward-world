#pragma once

#include "../UiElement.h"
#include <string>

namespace ui {

// ButtonModal-specific properties
struct ButtonModalProps {
  std::string text;
  bool isSelected = false;
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
