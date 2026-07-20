#pragma once

#include "../../UiElement.h"
#include "../TextParagraph.h"
#include "bmin/String.h"
#include "ui/colors.h"

namespace ui {

// ButtonTextWrap-specific properties
struct ButtonTextWrapProps {
  int verticalPadding = 0; // Padding added to top and bottom
  int horizontalPadding = 0; // Padding added to left and right
  bool isSelected = false;
  // Forwarded to the internal TextParagraph (width wraps text; height grows to fit).
  TextParagraphProps textParagraph;
};

// ButtonTextWrap element - renders a clickable quad with wrapped text that changes color on hover
// Uses Position, Size, Scale from BaseStyle
class ButtonTextWrap : public UiElement {
private:
  ButtonTextWrapProps props;
  bool isInHoverMode = false;
  bool isInActiveMode = false;

public:
  bool isActive = false;
  ButtonTextWrap(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ButtonTextWrap() override = default;

  // Setters and getters for button-specific properties
  void setProps(const ButtonTextWrapProps& _props);
  ButtonTextWrapProps& getProps();
  const ButtonTextWrapProps& getProps() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
