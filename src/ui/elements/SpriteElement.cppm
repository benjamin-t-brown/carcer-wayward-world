module;
#include <cstddef>
#include <cstdint>
#include <utility>
#include <stdexcept>

export module carcer.ui.elements:SpriteElement;
export import bmin.containers;
import bmin.string_interop;
export import carcer.ui.core;
import sdl2w;
import carcer.ui.core;
#include "macros.h"

export {

// --- from ui/elements/SpriteElement.h ---
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

} // export

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
