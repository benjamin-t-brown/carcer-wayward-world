#include "ButtonModal.h"
#include "../TextLine.h"
#include "ui/elements/OutsetRectangle.h"

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
  style.textAlign = TextAlign::CENTER;
  setBaseFontConfig(style, BaseFontConfig::MODAL_BUTTON);
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

  auto rect = new OutsetRectangle(window);
  auto& rectStyle = rect->getStyle();
  rectStyle.x = style.x;
  rectStyle.y = style.y;
  rectStyle.width = style.width;
  rectStyle.height = style.height;
  rectStyle.scale = style.scale;

  auto& rectProps = rect->getProps();
  if (isInActiveMode) {
    rectProps.borderSize = 0;
  } else {
    rectProps.borderSize = 2;
  }
  rectProps.color = props.bgColor;
  rectProps.colorTopRight = props.bgColorTopRight;
  rectProps.colorBottomLeft = props.bgColorBottomLeft;
  rect->setProps(rectProps);

  addChild(rect);

  auto textLine = new TextLine(window, this);
  auto& textStyle = textLine->getStyle();
  setBaseFontConfig(textStyle, BaseFontConfig::MODAL_BUTTON);
  textStyle.x = style.x + style.width * style.scale / 2;
  textStyle.y = style.y + style.height * style.scale / 2;
  if (isInActiveMode && !props.isSelected) {
    textStyle.x -= 1;
  }
  textStyle.textAlign = TextAlign::CENTER;
  textStyle.scale = 1.f;
  textStyle.fontColor = style.fontColor;
  textStyle.fontSize = style.fontSize;
  TextLineProps textLineProps;
  textLineProps.textBlocks = {TextBlock{props.text}};
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
}

} // namespace ui