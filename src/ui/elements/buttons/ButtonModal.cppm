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

export module carcer.ui.elements:ButtonModal;
export import bmin.containers;
import bmin.string_interop;
export import carcer.ui.TextStyle;
export import carcer.ui.UiElement;
import sdl2w;
import :OutsetRectangle;
import :TextLine;
#include "macros.h"

export {

// --- from ui/elements/buttons/ButtonModal.h ---
namespace ui {

// ButtonModal-specific properties
struct ButtonModalProps {
  bmin::String text;
  bool isSelected = false;
  int width = 80;
  int height = 32;

  SDL_Color bgColor = Colors::ButtonModalGrey1;
  SDL_Color bgColorTopRight = Colors::ButtonModalGrey2;
  SDL_Color bgColorBottomLeft = Colors::ButtonModalGrey3;

  FontFamily fontFamily = FontFamily::TEXT;
  sdl2w::TextSize fontSize = sdl2w::TEXT_SIZE_20;
  SDL_Color fontColor = Colors::White;
};

// ButtonModal element - renders a clickable button typically used inside modal windows
// Position/scale via setPos/setScale; size via props.width/height → build
class ButtonModal : public UiElement {
private:
  ButtonModalProps props;
  bool isInHoverMode = false;
  bool isInActiveMode = false;

public:
  bool isActive = false;
  ButtonModal(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ButtonModal() override = default;

  // Setters and getters for button-specific properties
  void setProps(const ButtonModalProps& _props);
  ButtonModalProps& getProps();
  const ButtonModalProps& getProps() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

namespace ui {

class ButtonModalDefaultObserver : public UiEventObserver {
  ButtonModal* buttonModal;

public:
  ButtonModalDefaultObserver(ButtonModal* _buttonModal) : buttonModal(_buttonModal) {}
  ~ButtonModalDefaultObserver() override = default;
  void onMouseDown(int x, int y, int button) override { buttonModal->isActive = true; }
  void onMouseUp(int x, int y, int button) override { buttonModal->isActive = false; }
};

ButtonModal::ButtonModal(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  addEventObserver(new ButtonModalDefaultObserver(this));
  TextFontProps font;
  setBaseFontConfig(font, BaseFontConfig::MODAL_BUTTON);
  props.fontFamily = font.fontFamily;
  props.fontSize = font.fontSize;
  props.fontColor = font.fontColor;
  shouldPropagateEventsToChildren = false;
}

void ButtonModal::setProps(const ButtonModalProps& _props) {
  props = _props;
  build();
}

ButtonModalProps& ButtonModal::getProps() { return props; }

const ButtonModalProps& ButtonModal::getProps() const { return props; }

void ButtonModal::build() {
  children.clear();

  style.width = props.width;
  style.height = props.height;

  auto rect = new OutsetRectangle(window);
  rect->setPos(style.x, style.y);
  rect->setScale(style.scale);

  OutsetRectangleProps rectProps;
  if (isInActiveMode) {
    rectProps.borderSize = 0;
  } else {
    rectProps.borderSize = 2;
  }
  rectProps.width = props.width;
  rectProps.height = props.height;
  rectProps.color = props.bgColor;
  rectProps.colorTopRight = props.bgColorTopRight;
  rectProps.colorBottomLeft = props.bgColorBottomLeft;
  rect->setProps(rectProps);

  addChild(rect);

  auto textLine = new TextLine(window, this);
  int textX = style.x + style.width * style.scale / 2;
  int textY = style.y + style.height * style.scale / 2;
  if (isInActiveMode && !props.isSelected) {
    textX -= 1;
  }
  textLine->setPos(textX, textY);
  textLine->setScale(1.f);
  TextLineProps textLineProps;
  textLineProps.fontFamily = props.fontFamily;
  textLineProps.fontSize = props.fontSize;
  textLineProps.fontColor = props.fontColor;
  textLineProps.textAlign = TextAlign::CENTER;
  textLineProps.textBlocks.pushBack(TextBlock{props.text});
  textLine->setProps(textLineProps);
  addChild(textLine);
}

void ButtonModal::render(int dt) {
  if (isActive) {
    if (!isInActiveMode) {
      isInActiveMode = true;
      build();
    }
  } else {
    if (isInActiveMode) {
      isInActiveMode = false;
      build();
    }
  }

  if (props.isSelected) {
    auto& draw = window->getDraw();
    auto scaledWidth = static_cast<int>(style.width * style.scale);
    auto scaledHeight = static_cast<int>(style.height * style.scale);
    int borderSize = 2;

    draw.drawRect(style.x - borderSize,
                  style.y - borderSize,
                  scaledWidth + borderSize * 2,
                  scaledHeight + borderSize * 2,
                  props.bgColor);
  }
  UiElement::render(dt);

  if (isActive) {
    auto& draw = window->getDraw();
    auto scaledWidth = static_cast<int>(style.width * style.scale);
    auto scaledHeight = static_cast<int>(style.height * style.scale);
    draw.drawRect(style.x, style.y, scaledWidth, scaledHeight, SDL_Color{0, 0, 0, 25});
  }
}

} // namespace ui
