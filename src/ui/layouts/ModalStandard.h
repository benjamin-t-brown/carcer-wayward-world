#pragma once

#include "../UiElement.h"
#include "ui/SdlPixels.h" // IWYU pragma: keep
#include <string>

namespace ui {

// ModalStandard layout properties
struct ModalStandardProps {
  SDL_Color contentBackgroundColor = Colors::White;
  std::string decorationSprite = "";
  std::string iconSprite;
};

// ModalStandard layout - renders a modal with background, border, title, subtitle, close
// button, and children Uses Position, Size, Scale from BaseStyle
class ModalStandard : public UiElement {
private:
  ModalStandardProps props;

public:
  ModalStandard(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ModalStandard() override = default;

  // Setters and getters for layout-specific properties
  void setProps(const ModalStandardProps& _props);
  ModalStandardProps& getProps();
  const ModalStandardProps& getProps() const;

  void setTitleElement(UiElement* _titleElement);
  UiElement* getTitleElement();
  UiElement* getCloseButtonElement();

  const std::pair<int, int> getSubTitleDims();
  const std::pair<int, int> getSubTitleLocation();
  const std::pair<int, int> getContentDims();
  const std::pair<int, int> getContentLocation();

  void build() override;
  void render(int dt) override;
};

} // namespace ui
