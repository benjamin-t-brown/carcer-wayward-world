#pragma once

#include "../UiElement.h"
#include <string>

namespace ui {

// Sprite element - renders a stylized sprite
// Uses Position, Size, Scale from BaseStyle
class Sprite : public UiElement {
private:
  std::string spriteName;

public:
  Sprite(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~Sprite() override = default;

  void setSpriteName(const std::string& name);
  const std::string& getSpriteName() const;

  void build() override;
  void render() override;
};

} // namespace ui

