#include "SectionDropShadow.h"
#include "ui/colors.h"
#include "ui/elements/Quad.h"

namespace ui {

SectionDropShadow::SectionDropShadow(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  shouldPropagateEventsToChildren = true;
}

void SectionDropShadow::setProps(const SectionDropShadowProps& _props) {
  props = _props;
  build();
}

SectionDropShadowProps& SectionDropShadow::getProps() { return props; }

const SectionDropShadowProps& SectionDropShadow::getProps() const { return props; }

void SectionDropShadow::addChild(std::unique_ptr<UiElement> child) {
  if (children.size() > 0) {
    children[0]->getChildren().push_back(std::move(child));
  }
}

void SectionDropShadow::build() {
  auto q = std::make_unique<ui::Quad>(window);
  ui::BaseStyle quadStyle;
  quadStyle.x = style.x;
  quadStyle.y = style.y;
  quadStyle.width = style.width;
  quadStyle.height = style.height;
  q->setStyle(quadStyle);
  ui::QuadProps quadProps;
  quadProps.bgColor = props.backgroundColor;
  quadProps.borderColor = Colors::Transparent;
  quadProps.borderSize = 0;
  q->setProps(quadProps);
  children.push_back(std::move(q));
}

void SectionDropShadow::render() {
  auto& draw = window->getDraw();

  auto scaledWidth = static_cast<int>(style.width * style.scale);
  auto scaledHeight = static_cast<int>(style.height * style.scale);

  // Render the black shadow first (behind the white background)
  int shadowX = style.x + props.shadowOffsetX;
  int shadowY = style.y + props.shadowOffsetY;
  int shadowWidth = scaledWidth;
  int shadowHeight = scaledHeight;

  // shadow
  draw.drawRect(shadowX, shadowY, shadowWidth, shadowHeight, props.shadowColor);
  // Render border if specified
  if (props.borderSize > 0) {
    // Draw border outline
    draw.drawRect(style.x - props.borderSize,
                  style.y - props.borderSize,
                  scaledWidth + 2 * props.borderSize,
                  scaledHeight + 2 * props.borderSize,
                  props.shadowColor);
  }

  // Render child elements on top
  UiElement::render();
}

} // namespace ui
