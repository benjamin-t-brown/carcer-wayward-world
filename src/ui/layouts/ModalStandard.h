#pragma once

#include "../UiElement.h"
#include <SDL2/SDL_pixels.h>
#include <string>
#include <memory>

namespace ui {

// ModalStandard layout properties
struct ModalStandardProps {
  SDL_Color backgroundColor = Colors::White;
  std::string decorationSprite = "";
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

  // Helper methods for child management
  UiElement* getChildById(const std::string& id);
  void removeChildById(const std::string& id);

  void setTitleElement(std::unique_ptr<UiElement> _titleElement);
  UiElement* getTitleElement();
  void setSubtitleElement(std::unique_ptr<UiElement> _subtitleElement);
  UiElement* getSubtitleElement();
  void setContentElement(std::unique_ptr<UiElement> _contentElement);
  UiElement* getContentElement();
  UiElement* getCloseButtonElement();

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
