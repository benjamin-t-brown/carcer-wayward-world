#pragma once

#include "../UiElement.h"
#include "lib/sdl2w/Draw.h"
#include <string>

namespace ui {

// Sprite element - renders a stylized sprite
// Uses Position, Size, Scale from BaseStyle
class SpriteElement : public UiElement {
private:
  sdl2w::Sprite sprite;

public:
  SpriteElement(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~SpriteElement() override = default;

  void setSprite(const std::string& name);
  const sdl2w::Sprite& getSprite() const;

  void build() override;
  void render() override;
};

} // namespace ui
