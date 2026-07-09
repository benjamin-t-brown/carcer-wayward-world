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
}

} // namespace ui
