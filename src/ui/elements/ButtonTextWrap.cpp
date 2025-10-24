#include "ButtonTextWrap.h"
#include "TextParagraph.h"
#include "ui/colors.h"
#include <memory>

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
  addEventObserver(std::make_unique<ButtonTextWrapDefaultObserver>(this));
  style.textAlign = TextAlign::CENTER;
  style.fontSize = sdl2w::TEXT_SIZE_16;
  style.fontColor = Colors::Black;
  shouldPropagateEventsToChildren = false;
}

void ButtonTextWrap::setProps(const ButtonTextWrapProps& _props) {
  props = _props;
  build();
}

ButtonTextWrapProps& ButtonTextWrap::getProps() { return props; }

const ButtonTextWrapProps& ButtonTextWrap::getProps() const { return props; }

const std::pair<int, int> ButtonTextWrap::getDims() const {
  if (children.size() > 0) {
    auto pChild = children[0].get();
    auto pair = pChild->getDims();
    return {pair.first + props.horizontalPadding * 2,
            pair.second + props.verticalPadding * 2};
  }
  return {style.width + props.horizontalPadding * 2, props.verticalPadding * 2};
}

void ButtonTextWrap::build() {
  children.clear();

  auto textParagraph = std::make_unique<TextParagraph>(window, this);
  BaseStyle textStyle;
  textStyle.x = style.x + props.horizontalPadding * style.scale;
  textStyle.y = style.y + props.verticalPadding * style.scale;
  textStyle.width = style.width * style.scale;
  textStyle.textAlign = TextAlign::LEFT_TOP;
  textStyle.fontSize = style.fontSize;
  textStyle.fontFamily = style.fontFamily;
  textStyle.fontColor = style.fontColor;
  textStyle.scale = 1;
  textParagraph->setStyle(textStyle);
  TextParagraphProps textParagraphProps;
  ui::TextBlock block;
  block.text = props.text;
  block.fontColor = style.fontColor;
  textParagraphProps.textBlocks.push_back(block);
  textParagraph->setProps(textParagraphProps);

  children.push_back(std::move(textParagraph));
}

void ButtonTextWrap::render() {
  SDL_Color bgColor = Colors::Transparent;

  if (isActive) {
    bgColor = SDL_Color{0, 0, 0, 100};
  } else if (isHovered) {
    bgColor = SDL_Color{0, 0, 0, 50};
  }

  auto& draw = window->getDraw();
  auto dims = getDims();
  int borderSize = 0;
  draw.drawRect(style.x - borderSize,
                style.y - borderSize,
                dims.first + borderSize * 2,
                dims.second + borderSize * 2,
                bgColor);

  UiElement::render();
}

} // namespace ui
