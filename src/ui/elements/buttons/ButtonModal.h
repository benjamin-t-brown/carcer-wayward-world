#pragma once

#include "../../UiElement.h"
#include "bmin/String.h"
#include "ui/TextStyle.h"

namespace ui {

// ButtonModal-specific properties
struct ButtonModalProps {
  bmin::String text;
  bool isSelected = false;
  int width = 80;
  int height = 32;

  SDL_Color bgColor = Colors::ButtonModalGrey1;
  SDL_Color bgColorTopRight = Colors::ButtonModalGrey2;
  SDL_Color bgColorBottomLeft = Colors::ButtonModalGrey3;

  FontFamily fontFamily = FontFamily::TEXT;
  sdl2w::TextSize fontSize = sdl2w::TEXT_SIZE_20;
  SDL_Color fontColor = Colors::White;
};

// ButtonModal element - renders a clickable button typically used inside modal windows
// Position/scale via setPos/setScale; size via props.width/height → build
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
