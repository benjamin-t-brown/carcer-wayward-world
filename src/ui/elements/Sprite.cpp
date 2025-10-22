#include "Sprite.h"
#include "lib/sdl2w/Draw.h"

namespace ui {

Sprite::Sprite(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

void Sprite::setSpriteName(const std::string& name) {
  spriteName = name;
  build();
}

const std::string& Sprite::getSpriteName() const { return spriteName; }

void Sprite::build() {
  // Build logic can be extended here if needed
  // Called when sprite name or style changes
}

void Sprite::render() {
  if (spriteName.empty()) {
    return;
  }

  auto& draw = window->getDraw();
  auto& store = window->getStore();

  auto& spriteData = store.getSprite(spriteName);

  // Set up render parameters
  sdl2w::RenderableParamsEx params;
  params.x = style.x;
  params.y = style.y;
  params.w = style.width;
  params.h = style.height;
  params.scale = {style.scale, style.scale};
  params.centered = false;

  // Draw the sprite
  draw.drawSprite(spriteData, params);

  // Render children (if any)
  UiElement::render();
}

} // namespace ui

