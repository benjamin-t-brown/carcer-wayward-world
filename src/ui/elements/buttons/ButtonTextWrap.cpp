#include "ButtonTextWrap.h"
#include "ui/colors.h"
#include <cmath>

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
