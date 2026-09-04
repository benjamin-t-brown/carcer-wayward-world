module;
#include <cstddef>
#include <cstdint>
#include <utility>
#include <optional>
#if __has_include(<SDL.h>)
#include <SDL.h>
#include <SDL_pixels.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_pixels.h>
#endif
#include <algorithm>

export module carcer.ui.elements:ButtonList;
export import bmin.containers;
import bmin.string_interop;
export import carcer.ui.UiElement;
export import carcer.ui.colors;
export import :ButtonScroll;
import sdl2w;
import carcer.ui.TextStyle;
import :OutsetRectangle;
import :TextLine;
#include "macros.h"

export {

// --- from ui/elements/buttons/ButtonList.h ---
namespace ui {

struct ButtonListProps {
  bmin::String text;
  std::optional<ScrollDirection> arrow;
  bool isSelected = false;
  int width = 14;
  int height = 14;

  SDL_Color bgColor = Colors::ButtonModalGrey1;
  SDL_Color bgColorTopRight = Colors::ButtonModalGrey2;
  SDL_Color bgColorBottomLeft = Colors::ButtonModalGrey3;
  SDL_Color arrowColor = Colors::White;
  SDL_Color fontColor = Colors::White;
};

// ButtonList - square button for list rows (modal styling for now)
class ButtonList : public UiElement {
private:
  ButtonListProps props;
  bool isInActiveMode = false;

public:
  static constexpr int defaultLogicalSize = 14;

  bool isActive = false;
  ButtonList(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ButtonList() override = default;

  static int yForListRow(int rowHeight, int btnLogicalSize, float scale);

  void setProps(const ButtonListProps& _props);
  ButtonListProps& getProps();
  const ButtonListProps& getProps() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

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

  style.width = props.width;
  style.height = props.height;

  auto rect = new OutsetRectangle(window);
  rect->setPos(style.x, style.y);
  rect->setScale(style.scale);

  OutsetRectangleProps rectProps;
  rectProps.width = props.width;
  rectProps.height = props.height;
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
    int textX = style.x + style.width * style.scale / 2;
    int textY = style.y + style.height * style.scale / 2;
    if (isInActiveMode && !props.isSelected) {
      textX -= 1;
    }
    textLine->setPos(textX, textY);
    textLine->setScale(1.f);
    TextLineProps listTextProps;
    TextFontProps font;
    setBaseFontConfig(font, BaseFontConfig::MODAL_BUTTON);
    listTextProps.fontFamily = font.fontFamily;
    listTextProps.fontSize = font.fontSize;
    listTextProps.fontColor = props.fontColor;
    listTextProps.textAlign = TextAlign::CENTER;
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
