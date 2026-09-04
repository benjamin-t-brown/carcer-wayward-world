module;
#include <cstddef>
#include <cstdint>
#include <utility>
#include <cmath>
#if __has_include(<SDL.h>)
#include <SDL.h>
#include <SDL_pixels.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_pixels.h>
#endif

export module carcer.ui.elements:ButtonTextWrap;
export import bmin.containers;
import bmin.string_interop;
export import carcer.ui.core;
export import :TextParagraph;
import sdl2w;
import carcer.ui.core;
#include "macros.h"

export {

// --- from ui/elements/buttons/ButtonTextWrap.h ---
namespace ui {

// ButtonTextWrap-specific properties
struct ButtonTextWrapProps {
  int verticalPadding = 0; // Padding added to top and bottom
  int horizontalPadding = 0; // Padding added to left and right
  bool isSelected = false;
  // Forwarded to the internal TextParagraph (width wraps text; height grows to fit).
  TextParagraphProps textParagraph;
};

// ButtonTextWrap element - renders a clickable quad with wrapped text that changes color on hover
// Uses Position, Size, Scale from BaseStyle
class ButtonTextWrap : public UiElement {
private:
  ButtonTextWrapProps props;
  bool isInHoverMode = false;
  bool isInActiveMode = false;

public:
  bool isActive = false;
  ButtonTextWrap(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ButtonTextWrap() override = default;

  // Setters and getters for button-specific properties
  void setProps(const ButtonTextWrapProps& _props);
  ButtonTextWrapProps& getProps();
  const ButtonTextWrapProps& getProps() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

namespace ui {

class ButtonTextWrapDefaultObserver : public UiEventObserver {
  ButtonTextWrap* buttonTextWrap;

public:
  ButtonTextWrapDefaultObserver(ButtonTextWrap* _buttonTextWrap)
      : buttonTextWrap(_buttonTextWrap) {}
  ~ButtonTextWrapDefaultObserver() override = default;
  void onMouseDown(int x, int y, int button) override { buttonTextWrap->isActive = true; }
  void onMouseUp(int x, int y, int button) override { buttonTextWrap->isActive = false; }
};

ButtonTextWrap::ButtonTextWrap(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  addEventObserver(new ButtonTextWrapDefaultObserver(this));
  TextFontProps font;
  setBaseFontConfig(font, BaseFontConfig::MODAL_TEXT);
  props.textParagraph.fontFamily = font.fontFamily;
  props.textParagraph.fontSize = font.fontSize;
  props.textParagraph.fontColor = Colors::Black;
  shouldPropagateEventsToChildren = false;
}

void ButtonTextWrap::setProps(const ButtonTextWrapProps& _props) {
  props = _props;
  build();
}

ButtonTextWrapProps& ButtonTextWrap::getProps() { return props; }

const ButtonTextWrapProps& ButtonTextWrap::getProps() const { return props; }

void ButtonTextWrap::build() {
  children.clear();

  const float scale = style.scale > 0.f ? style.scale : 1.f;
  style.width = props.textParagraph.width + props.horizontalPadding * 2;

  auto textParagraph = new TextParagraph(window, this);
  textParagraph->setPos(style.x + static_cast<int>(props.horizontalPadding * scale),
                        style.y + static_cast<int>(props.verticalPadding * scale));
  textParagraph->setScale(scale);
  textParagraph->setProps(props.textParagraph);

  addChild(textParagraph);

  const int paragraphHeightScaled = textParagraph->getDims().second;
  style.height = static_cast<int>(std::round(paragraphHeightScaled / scale)) +
                 props.verticalPadding * 2;
}

void ButtonTextWrap::render(int dt) {
  SDL_Color bgColor = Colors::Transparent;

  if (isActive) {
    bgColor = SDL_Color{0, 0, 0, 25};
  } else if (isHovered) {
    // bgColor = SDL_Color{0, 0, 0, 50};
  }

  auto& draw = window->getDraw();
  auto dims = getDims();
  int borderSize = 0;
  draw.drawRect(style.x - borderSize,
                style.y - borderSize,
                dims.first + borderSize * 2,
                dims.second + borderSize * 2,
                bgColor);

  UiElement::render(dt);
}

} // namespace ui
