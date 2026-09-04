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
#include <typeinfo>
#include <typeindex>

export module carcer.ui.components.FloatingNotification;
export import bmin.containers;
import bmin.string_interop;
export import carcer.state;
export import carcer.ui.core;
import sdl2w;
import carcer.ui.BorderDropShadow;
import carcer.ui.core;
import carcer.ui.elements;
#include "macros.h"

export {

// --- from ui/components/FloatingNotification.h ---
namespace ui {

struct FloatingNotificationProps {
  bmin::String id;
  bmin::String message;
  state::UiFloatingNotificationType type = state::UiFloatingNotificationType::INFO;
};

class FloatingNotification : public UiElement {
private:
  static constexpr int kHorizontalPadding = 16;
  static constexpr int kVerticalPadding = 8;

  FloatingNotificationProps props;

  SDL_Color getTextColor() const;

public:
  FloatingNotification(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~FloatingNotification() override = default;

  void setProps(const FloatingNotificationProps& _props);
  const FloatingNotificationProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

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

  TextFontProps font;
  setBaseFontConfig(font, BaseFontConfig::MODAL_TEXT);

  auto textLine = new TextLine(window, this);
  textLine->setPos(style.x, style.y);
  textLine->setScale(1.f);
  TextLineProps notificationProps;
  notificationProps.fontFamily = font.fontFamily;
  notificationProps.fontSize = font.fontSize;
  notificationProps.fontColor = getTextColor();
  notificationProps.textAlign = TextAlign::CENTER;
  notificationProps.textBlocks.pushBack(
      {.text = props.message, .fontColor = getTextColor()});
  textLine->setProps(notificationProps);

  auto [textWidth, textHeight] = textLine->calculateTextDims();
  const int contentWidth = textWidth + kHorizontalPadding * 2;
  const int contentHeight = textHeight + kVerticalPadding * 2;

  style.width = contentWidth;
  style.height = contentHeight;

  auto border = new BorderDropShadow(window, this);
  border->setPos(style.x, style.y);
  border->setProps(BorderDropShadowProps{
      .width = contentWidth,
      .height = contentHeight,
      .backgroundColor = Colors::White,
      .shadowColor = Colors::Black,
      .shadowOffsetX = -4,
      .shadowOffsetY = 4,
      .borderSize = 2,
  });

  // TextLine renders in screen space (sibling of border), not inside the panel Quad.
  textLine->setPos(style.x + contentWidth / 2, style.y + contentHeight / 2);

  addChild(border);
  addChild(textLine);
}

void FloatingNotification::render(int dt) { UiElement::render(dt); }

} // namespace ui
