#pragma once

#include "../UiElement.h"
#include "ui/SdlPixels.h" // IWYU pragma: keep
#include <string>

namespace ui {

// ModalSmall layout properties
struct ModalSmallProps {
  SDL_Color backgroundColor = Colors::White;
  std::string iconSprite = "";
  bool enableCloseButton = true;
};

// ModalSmall layout - renders a small modal with background, border, title, subtitle, close
// button, and children Uses Position, Size, Scale from BaseStyle
class ModalSmall : public UiElement {
private:
  ModalSmallProps props;

  int getScaledButtonsAreaHeight();

public:
  static const int BUTTONS_AREA_HEIGHT = 50;

  ModalSmall(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ModalSmall() override = default;

  // Setters and getters for layout-specific properties
  void setProps(const ModalSmallProps& _props);
  ModalSmallProps& getProps();
  const ModalSmallProps& getProps() const;

  void setTitleElement(UiElement* _titleElement);
  UiElement* getTitleElement();
  UiElement* getCloseButtonElement();
  const std::pair<int, int> getContentDims();
  const std::pair<int, int> getContentLocation();
  const std::pair<int, int> getButtonsDims();
  const std::pair<int, int> getButtonsLocation();

  void build() override;
  void render(int dt) override;
};

} // namespace ui
