#include "SpriteElement.h"
#include "bmin/StringInterop.h"
#include "sdl2w/Draw.h"
#include "sdl2w/Logger.h"

namespace ui {

SpriteElement::SpriteElement(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

void SpriteElement::setProps(const SpriteElementProps& _props) {
  props = _props;
  build();
}

SpriteElementProps& SpriteElement::getProps() { return props; }

const SpriteElementProps& SpriteElement::getProps() const { return props; }

void SpriteElement::setSprite(const bmin::String& name) {
  props.spriteName = name;
  build();
}

const sdl2w::Sprite& SpriteElement::getSprite() const { return sprite; }

void SpriteElement::build() {
  style.width = props.width;
  style.height = props.height;
  if (props.spriteName.empty()) {
    sprite = sdl2w::Sprite{};
    return;
  }
  try {
    sprite = window->getStore().getSprite(bmin::toStringView(props.spriteName));
  } catch (const std::runtime_error& e) {
    LOG_LINE(ERROR)
        << (bmin::String("[ui] ERROR When setting Sprite for ui sprite element. Cannot get Sprite '") +
            props.spriteName.cStr() + "' because it has not been loaded.")
               .cStr()
        << LOG_ENDL;
    throw std::runtime_error(
        (bmin::String("Failed to get Sprite '") + props.spriteName.cStr() + "'").cStr());
  }
}

void SpriteElement::render(int dt) {
  auto& draw = window->getDraw();

  if (sprite.name.empty()) {
    return;
  }

  sdl2w::RenderableParamsEx params;
  params.x = style.x;
  params.y = style.y;
  params.w = style.width;
  params.h = style.height;
  params.scale = {style.scale, style.scale};
  params.centered = false;

  draw.drawSprite(sprite, params);
}

} // namespace ui
