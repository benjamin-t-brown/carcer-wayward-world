#include "ButtonList.h"
#include "../TextLine.h"
#include "ui/elements/OutsetRectangle.h"
#include <algorithm>

namespace ui {

class ButtonListDefaultObserver : public UiEventObserver {
  ButtonList* buttonList;

public:
  ButtonListDefaultObserver(ButtonList* _buttonList) : buttonList(_buttonList) {}
  ~ButtonListDefaultObserver() override = default;
  void onMouseDown(int x, int y, int button) override { buttonList->isActive = true; }
  void onMouseUp(int x, int y, int button) override { buttonList->isActive = false; }
};

int ButtonList::yForListRow(int rowHeight, int btnLogicalSize, float scale) {
  const int scaledBtnSize = static_cast<int>(btnLogicalSize * scale);
  return (rowHeight - scaledBtnSize) / 2;
}

ButtonList::ButtonList(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  addEventObserver(new ButtonListDefaultObserver(this));
  shouldPropagateEventsToChildren = false;
  style.width = defaultLogicalSize;
  style.height = defaultLogicalSize;
}

void ButtonList::setProps(const ButtonListProps& _props) {
  props = _props;
  build();
}

ButtonListProps& ButtonList::getProps() { return props; }

const ButtonListProps& ButtonList::getProps() const { return props; }

void ButtonList::build() {
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

  if (!props.arrow.has_value() && !props.text.empty()) {
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
    TextLineProps listTextProps;
    listTextProps.textBlocks.pushBack(TextBlock{props.text});
    textLine->setProps(listTextProps);
    addChild(textLine);
  }
}

void ButtonList::render(int dt) {
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

  if (!props.arrow.has_value()) {
    return;
  }

  auto scaledX = static_cast<int>(style.x);
  auto scaledY = static_cast<int>(style.y);
  auto scaledWidth = static_cast<int>(style.width * style.scale);
  auto scaledHeight = static_cast<int>(style.height * style.scale);
  auto centerX = scaledX + scaledWidth / 2;
  auto centerY = scaledY + scaledHeight / 2;
  const auto arrowBasis = std::max(scaledWidth, scaledHeight);
  const auto arrowLength = std::max(arrowBasis / 4, 5) - 2;
  if (isInActiveMode) {
    centerX -= static_cast<int>(style.scale);
  }

  auto& draw = window->getDraw();
  if (*props.arrow == ScrollDirection::UP) {
    draw.drawLine({centerX - arrowLength, centerY},
                  {centerX, centerY - arrowLength},
                  style.scale,
                  props.arrowColor);
    draw.drawLine({centerX, centerY + arrowLength},
                  {centerX, centerY - arrowLength},
                  style.scale,
                  props.arrowColor);
    draw.drawLine({centerX + arrowLength, centerY},
                  {centerX, centerY - arrowLength},
                  style.scale,
                  props.arrowColor);
  } else if (*props.arrow == ScrollDirection::DOWN) {
    draw.drawLine({centerX - arrowLength, centerY},
                  {centerX, centerY + arrowLength},
                  style.scale,
                  props.arrowColor);
    draw.drawLine({centerX, centerY - arrowLength},
                  {centerX, centerY + arrowLength},
                  style.scale,
                  props.arrowColor);
    draw.drawLine({centerX + arrowLength, centerY},
                  {centerX, centerY + arrowLength},
                  style.scale,
                  props.arrowColor);
  }
}

} // namespace ui
