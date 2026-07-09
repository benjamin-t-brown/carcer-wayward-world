#include "FloatingNotification.h"
#include "ui/colors.h"
#include "ui/components/borders/BorderDropShadow.h"
#include "ui/elements/TextLine.h"

namespace ui {

FloatingNotification::FloatingNotification(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  shouldPropagateEventsToChildren = false;
}

SDL_Color FloatingNotification::getTextColor() const {
  switch (props.type) {
  case state::UiFloatingNotificationType::WARNING:
    return Colors::Black;
  case state::UiFloatingNotificationType::ERROR:
    return Colors::Red;
  case state::UiFloatingNotificationType::INFO:
  default:
    return Colors::Blue;
  }
}

void FloatingNotification::setProps(const FloatingNotificationProps& _props) {
  props = _props;
  build();
}

const FloatingNotificationProps& FloatingNotification::getProps() const { return props; }

const std::pair<int, int> FloatingNotification::getDims() const {
  return {style.width, style.height};
}

void FloatingNotification::build() {
  children.clear();

  auto textLine = new TextLine(window, this);
  auto& textStyle = textLine->getStyle();
  setBaseFontConfig(textStyle, BaseFontConfig::MODAL_TEXT);
  textStyle.fontColor = getTextColor();
  textStyle.textAlign = TextAlign::CENTER;
  TextLineProps notificationProps;
  notificationProps.textBlocks.pushBack(
      {.text = props.message, .fontColor = getTextColor()});
  textLine->setProps(notificationProps);

  auto [textWidth, textHeight] = textLine->calculateTextDims();
  const int contentWidth = textWidth + kHorizontalPadding * 2;
  const int contentHeight = textHeight + kVerticalPadding * 2;

  style.width = contentWidth;
  style.height = contentHeight;

  auto border = new BorderDropShadow(window, this);
  auto& borderStyle = border->getStyle();
  borderStyle.x = style.x;
  borderStyle.y = style.y;
  borderStyle.width = contentWidth;
  borderStyle.height = contentHeight;
  border->setProps(BorderDropShadowProps{
      .backgroundColor = Colors::White,
      .shadowColor = Colors::Black,
      .shadowOffsetX = -4,
      .shadowOffsetY = 4,
      .borderSize = 2,
  });

  // TextLine renders in screen space (sibling of border), not inside the panel Quad.
  textStyle.x = style.x + contentWidth / 2;
  textStyle.y = style.y + contentHeight / 2;
  textStyle.width = textWidth;
  textStyle.height = textHeight;
  textLine->build();

  addChild(border);
  addChild(textLine);
}

void FloatingNotification::render(int dt) { UiElement::render(dt); }

} // namespace ui
