#pragma once

#include "../UiElement.h"
#include "sdl2w/Draw.h"
#include "bmin/String.h"

namespace ui {

struct SpriteElementProps {
  int width = 0;
  int height = 0;
  bmin::String spriteName;
};

// Sprite element - renders a stylized sprite
// Position/scale via setPos/setScale; size/sprite via props → build
class SpriteElement : public UiElement {
private:
  SpriteElementProps props;
  sdl2w::Sprite sprite;

public:
  SpriteElement(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~SpriteElement() override = default;

  void setProps(const SpriteElementProps& _props);
  SpriteElementProps& getProps();
  const SpriteElementProps& getProps() const;

  // Convenience: sets sprite name and rebuilds
  void setSprite(const bmin::String& name);
  const sdl2w::Sprite& getSprite() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
