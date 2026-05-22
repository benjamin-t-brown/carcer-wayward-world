#pragma once

#include "../UiElement.h"
#include "ui/SdlPixels.h" // IWYU pragma: keep
#include <string>

namespace ui {

// ModalSmall layout properties
struct ModalSmallProps {
  SDL_Color backgroundColor = Colors::White;
  std::string iconSprite = "";
};

// ModalSmall layout - renders a small modal with background, border, title, subtitle, close
// button, and children Uses Position, Size, Scale from BaseStyle
class ModalSmall : public UiElement {
private:
  ModalSmallProps props;

public:
  ModalSmall(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ModalSmall() override = default;

  // Setters and getters for layout-specific properties
  void setProps(const ModalSmallProps& _props);
  ModalSmallProps& getProps();
  const ModalSmallProps& getProps() const;

  // Helper methods for child management
  UiElement* getChildById(const std::string& id) override;
  void removeChildById(const std::string& id);

  void setTitleElement(UiElement* _titleElement);
  UiElement* getTitleElement();
  void setContentElement(UiElement* _contentElement);
  UiElement* getContentElement();
  UiElement* getCloseButtonElement();

  void build() override;
  void render(int dt) override;
};

} // namespace ui
