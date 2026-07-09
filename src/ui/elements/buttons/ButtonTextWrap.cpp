#include "ButtonTextWrap.h"
#include "../TextParagraph.h"
#include "ui/colors.h"

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
  props.fontFamily = font.fontFamily;
  props.fontSize = font.fontSize;
  props.fontColor = Colors::Black;
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

  style.width = props.width;
  style.height = props.height;

  auto textParagraph = new TextParagraph(window, this);
  textParagraph->setPos(style.x + props.horizontalPadding * style.scale,
                        style.y + props.verticalPadding * style.scale);
  textParagraph->setScale(1.f);
  TextParagraphProps textParagraphProps;
  textParagraphProps.width = static_cast<int>(props.width * style.scale);
  textParagraphProps.fontFamily = props.fontFamily;
  textParagraphProps.fontSize = props.fontSize;
  textParagraphProps.fontColor = props.fontColor;
  textParagraphProps.textAlign = TextAlign::LEFT_TOP;
  ui::TextBlock block;
  block.text = props.text;
  block.fontColor = props.fontColor;
  textParagraphProps.textBlocks.pushBack(block);
  textParagraph->setProps(textParagraphProps);

  addChild(textParagraph);
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
