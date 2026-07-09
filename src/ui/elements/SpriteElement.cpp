#include "SpriteElement.h"
#include "lib/sdl2w/Draw.h"
#include "lib/sdl2w/Logger.h"

namespace ui {

SpriteElement::SpriteElement(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

void SpriteElement::setSprite(const String& name) {
  try {
    sprite = window->getStore().getSprite(bmin::toStringView(name));
  } catch (const std::runtime_error& e) {
    LOG_LINE(ERROR)
        << (String("[ui] ERROR When setting Sprite for ui sprite element. Cannot get Sprite '") +
            name.cStr() + "' because it has not been loaded.")
               .cStr()
        << LOG_ENDL;
    throw std::runtime_error((String("Failed to get Sprite '") + name.cStr() + "'").cStr());
  }
  build();
}

const sdl2w::Sprite& SpriteElement::getSprite() const { return sprite; }

void SpriteElement::build() {
  // noop
}

void SpriteElement::render(int dt) {
  auto& draw = window->getDraw();

  if (sprite.name.empty()) {
    return;
  }

  // Set up render parameters
  sdl2w::RenderableParamsEx params;
  params.x = style.x;
  params.y = style.y;
  params.w = style.width;
  params.h = style.height;
  params.scale = {style.scale, style.scale};
  params.centered = false;

  // Draw the sprite
  draw.drawSprite(sprite, params);
}

} // namespace ui
