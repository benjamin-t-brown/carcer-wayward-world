#include "BorderDropShadow.h"
#include "ui/elements/Quad.h"

namespace ui {

BorderDropShadow::BorderDropShadow(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  shouldPropagateEventsToChildren = true;
}

void BorderDropShadow::setProps(const BorderDropShadowProps& _props) {
  props = _props;
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
  std::vector<std::unique_ptr<UiElement>> preservedChildren;
  if (!children.empty()) {
    for (auto& child : children[0]->getChildren()) {
      preservedChildren.push_back(std::move(child));
    }
  }

  auto quad = std::make_unique<Quad>(window);
  auto& quadStyle = quad->getStyle();
  quadStyle.x = style.x;
  quadStyle.y = style.y;
  quadStyle.width = style.width;
  quadStyle.height = style.height;
  quadStyle.scale = style.scale;
  quad->setProps(QuadProps{
      .bgColor = props.backgroundColor,
      .borderColor = Colors::Transparent,
      .borderSize = 0,
  });

  for (auto& child : preservedChildren) {
    quad->addChild(child.release());
  }

  children.clear();
  children.push_back(std::move(quad));
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
