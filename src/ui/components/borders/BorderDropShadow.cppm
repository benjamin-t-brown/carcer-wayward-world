module;
#include <cstddef>
#include <cstdint>
#include <utility>
#if __has_include(<SDL.h>)
#include <SDL.h>
#include <SDL_pixels.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_pixels.h>
#endif
#include <algorithm>

export module carcer.ui.components:BorderDropShadow;
export import carcer.ui.core;
import sdl2w;
import carcer.ui.elements;
import bmin.string_interop;
#include "macros.h"

export {

// --- from ui/components/borders/BorderDropShadow.h ---
namespace ui {

struct BorderDropShadowProps {
  int width = 0;
  int height = 0;
  SDL_Color backgroundColor = Colors::White;
  SDL_Color shadowColor = Colors::Black;
  int shadowOffsetX = -8;
  int shadowOffsetY = 8;
  int borderSize = 2;
  bool isSelected = false;
};

// BorderDropShadow - panel with fill, offset drop shadow, and outline border.
// Content children are laid out in logical space inside an internal scaled Quad.
class BorderDropShadow : public UiElement {
  BorderDropShadowProps props;

public:
  BorderDropShadow(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~BorderDropShadow() override = default;

  void setProps(const BorderDropShadowProps& _props);
  BorderDropShadowProps& getProps();
  const BorderDropShadowProps& getProps() const;

  void addChild(UiElement* child) override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

namespace ui {

BorderDropShadow::BorderDropShadow(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  shouldPropagateEventsToChildren = true;
}

void BorderDropShadow::setProps(const BorderDropShadowProps& _props) {
  props = _props;
  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }
  build();
}

BorderDropShadowProps& BorderDropShadow::getProps() { return props; }

const BorderDropShadowProps& BorderDropShadow::getProps() const { return props; }

void BorderDropShadow::addChild(UiElement* child) {
  if (!children.empty()) {
    children[0]->addChild(child);
  }
}

void BorderDropShadow::build() {
  bmin::DynArray<bmin::UniquePtr<UiElement>> preservedChildren;
  if (!children.empty()) {
    for (auto& child : children[0]->getChildren()) {
      preservedChildren.pushBack(std::move(child));
    }
  }

  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }

  auto quad = bmin::makeUnique<Quad>(window);
  quad->setPos(style.x, style.y);
  quad->setScale(style.scale);
  quad->setProps(QuadProps{
      .width = style.width,
      .height = style.height,
      .bgColor = props.backgroundColor,
      .borderColor = Colors::Transparent,
      .borderSize = 0,
  });

  for (auto& child : preservedChildren) {
    quad->addChild(child.release());
  }

  children.clear();
  children.pushBack(bmin::UniquePtr<UiElement>(quad.release()));
}

void BorderDropShadow::render(int dt) {
  auto& draw = window->getDraw();

  const int scaledWidth = static_cast<int>(style.width * style.scale);
  const int scaledHeight = static_cast<int>(style.height * style.scale);

  const int shadowX = style.x + props.shadowOffsetX;
  const int shadowY = style.y + props.shadowOffsetY;
  draw.drawRect(shadowX, shadowY, scaledWidth, scaledHeight, props.shadowColor);

  if (props.borderSize > 0) {
    draw.drawRect(style.x - props.borderSize,
                  style.y - props.borderSize,
                  scaledWidth + 2 * props.borderSize,
                  scaledHeight + 2 * props.borderSize,
                  props.shadowColor);
  }

  UiElement::render(dt);
}

} // namespace ui
