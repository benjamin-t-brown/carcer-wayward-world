#include "ButtonTextWrap.h"
#include "../TextParagraph.h"
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
  addEventObserver(new ButtonTextWrapDefaultObserver(this));
  style.textAlign = TextAlign::CENTER;
  setBaseFontConfig(style, BaseFontConfig::MODAL_TEXT);
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
    return {pair.first + props.horizontalPadding * 2 * style.scale,
            pair.second + props.verticalPadding * 2 * style.scale};
  }
  return {style.width * style.scale + props.horizontalPadding * 2 * style.scale,
          style.height * style.scale + props.verticalPadding * 2 * style.scale};
}

void ButtonTextWrap::build() {
  children.clear();

  auto textParagraph = new TextParagraph(window, this);
  auto& textStyle = textParagraph->getStyle();
  textStyle.x = style.x + props.horizontalPadding * style.scale;
  textStyle.y = style.y + props.verticalPadding * style.scale;
  textStyle.width = style.width * style.scale;
  textStyle.textAlign = TextAlign::LEFT_TOP;
  textStyle.fontSize = style.fontSize;
  textStyle.fontFamily = style.fontFamily;
  textStyle.fontColor = style.fontColor;
  textStyle.scale = 1;
  TextParagraphProps textParagraphProps;
  ui::TextBlock block;
  block.text = props.text;
  block.fontColor = style.fontColor;
  textParagraphProps.textBlocks.push_back(block);
  textParagraph->setProps(textParagraphProps);

  addChild(textParagraph);
}

void ButtonTextWrap::render(int dt) {
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

  UiElement::render(dt);
}

} // namespace ui
