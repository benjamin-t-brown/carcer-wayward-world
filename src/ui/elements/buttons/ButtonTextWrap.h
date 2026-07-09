#pragma once

#include "../../UiElement.h"
#include "bmin/String.h"
#include "ui/TextStyle.h"
#include "ui/colors.h"

namespace ui {

// ButtonTextWrap-specific properties
struct ButtonTextWrapProps {
  bmin::String text;
  int width = 0;
  int height = 0;
  int verticalPadding = 0; // Padding added to top and bottom
  int horizontalPadding = 0; // Padding added to left and right
  bool isSelected = false;
  FontFamily fontFamily = FontFamily::TEXT;
  sdl2w::TextSize fontSize = sdl2w::TEXT_SIZE_16;
  SDL_Color fontColor = Colors::Black;
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

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
