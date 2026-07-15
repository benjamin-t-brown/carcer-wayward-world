#include "TouchMovePad.h"
#include "bmin/UniquePtr.h"
#include "ui/colors.h"
#include "ui/elements/OutsetRectangle.h"
#include "ui/elements/SpriteElement.h"
#include "ui/uiUtils.h"

namespace ui {

constexpr TouchMovePad::ButtonPlacement TouchMovePad::buttonPlacements[];

TouchMovePad::TouchMovePad(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  shouldPropagateEventsToChildren = true;
}

void TouchMovePad::setProps(const TouchMovePadProps& _props) {
  props = _props;
  build();
}

TouchMovePadProps& TouchMovePad::getProps() { return props; }

const TouchMovePadProps& TouchMovePad::getProps() const { return props; }

int TouchMovePad::getWideRowWidth() const {
  return cardButtonW + props.buttonGapH + halfButtonW + props.buttonGapH + cardButtonW;
}

int TouchMovePad::getNarrowRowWidth() const {
  return halfButtonW + props.buttonGapH + cardButtonW + props.buttonGapH + halfButtonW;
}

int TouchMovePad::getGridWidth() const { return getWideRowWidth(); }

int TouchMovePad::getGridHeight() const {
  return halfButtonH + props.buttonGapV + halfButtonH + props.buttonGapV + halfButtonH;
}

int TouchMovePad::getContentWidth() const {
  return getGridWidth() + props.padding * 2 + props.borderSize * 2;
}

int TouchMovePad::getBodyHeight() const {
  return getGridHeight() + props.padding * 2 + props.borderSize * 2;
}

int TouchMovePad::getContentHeight() const {
  return props.dragBarHeight + getBodyHeight();
}

int TouchMovePad::getBodyY() const { return style.y + props.dragBarHeight; }

std::pair<int, int> TouchMovePad::getButtonPosition(MoveDirection direction) const {
  const int gridW = getWideRowWidth();
  const int narrowW = getNarrowRowWidth();
  const int rowInset = (gridW - narrowW) / 2;

  const int bodyY = style.y + props.dragBarHeight;
  const int gridX = style.x + props.borderSize + props.padding;
  const int gridY = bodyY + props.borderSize + props.padding;
  const int rowStep = halfButtonH + props.buttonGapV;

  int buttonX = gridX;
  int buttonY = gridY;

  switch (direction) {
  case MoveDirection::UpLeft:
    buttonX = gridX + rowInset;
    break;
  case MoveDirection::Up:
    buttonX = gridX + rowInset + halfButtonW + props.buttonGapH;
    break;
  case MoveDirection::UpRight:
    buttonX = gridX + rowInset + halfButtonW + props.buttonGapH + cardButtonW +
              props.buttonGapH;
    break;
  case MoveDirection::Left:
    buttonY = gridY + rowStep;
    break;
  case MoveDirection::Wait:
    buttonX = gridX + cardButtonW + props.buttonGapH;
    buttonY = gridY + rowStep;
    break;
  case MoveDirection::Right:
    buttonX = gridX + cardButtonW + props.buttonGapH + halfButtonW + props.buttonGapH;
    buttonY = gridY + rowStep;
    break;
  case MoveDirection::DownLeft:
    buttonX = gridX + rowInset;
    buttonY = gridY + rowStep * 2;
    break;
  case MoveDirection::Down:
    buttonX = gridX + rowInset + halfButtonW + props.buttonGapH;
    buttonY = gridY + rowStep * 2;
    break;
  case MoveDirection::DownRight:
    buttonX = gridX + rowInset + halfButtonW + props.buttonGapH + cardButtonW +
              props.buttonGapH;
    buttonY = gridY + rowStep * 2;
    break;
  }

  return {buttonX, buttonY};
}

bool TouchMovePad::isInDragBar(int mouseX, int mouseY) const {
  const int dragBarX = style.x;
  const int dragBarY = style.y;
  const int dragBarW = static_cast<int>(getContentWidth() * style.scale);
  const int dragBarH = static_cast<int>(props.dragBarHeight * style.scale);
  return isInBounds(mouseX, mouseY, dragBarX, dragBarY, dragBarW, dragBarH);
}

void TouchMovePad::moveTo(int x, int y) {
  setPos(x, y);
  positionChildren();
}

void TouchMovePad::positionChildren() {
  if (auto* background = getChildById("background")) {
    background->setPos(style.x, getBodyY());
  }

  if (auto* dragBar = getChildById("dragBar")) {
    dragBar->setPos(style.x, style.y);
  }

  if (auto* dragHandle = getChildById("dragHandle")) {
    dragHandle->setPos(style.x + (getContentWidth() - borderButtonW) / 2,
                       style.y + (props.dragBarHeight - borderButtonH) / 2);
  }

  for (const auto& placement : buttonPlacements) {
    if (auto* button = getChildById(placement.id)) {
      const auto [buttonX, buttonY] = getButtonPosition(placement.direction);
      button->setPos(buttonX, buttonY);
    }
  }
}

void TouchMovePad::syncDragHandle() {
  auto* dragHandle = getChildById("dragHandle");
  if (dragHandle == nullptr) {
    return;
  }

  if (auto* sprite = dynamic_cast<SpriteElement*>(dragHandle)) {
    sprite->setSprite(isDragging ? "ui_border_buttons_4" : "ui_border_buttons_0");
  }
}

void TouchMovePad::startDrag(int mouseX, int mouseY) {
  isDragging = true;
  dragOffsetX = mouseX - style.x;
  dragOffsetY = mouseY - style.y;
  syncDragHandle();
  positionChildren();
}

void TouchMovePad::endDrag() {
  if (!isDragging) {
    return;
  }
  isDragging = false;
  syncDragHandle();
  positionChildren();
}

bool TouchMovePad::checkMouseDownEvent(int mouseX,
                                       int mouseY,
                                       int button,
                                       bmin::DynArray<UiElement*> additionalElements) {
  if (isInDragBar(mouseX, mouseY)) {
    startDrag(mouseX, mouseY);
    return true;
  }
  return UiElement::checkMouseDownEvent(mouseX, mouseY, button, additionalElements);
}

bool TouchMovePad::checkMouseUpEvent(int mouseX,
                                     int mouseY,
                                     int button,
                                     bmin::DynArray<UiElement*> additionalElements) {
  endDrag();
  return UiElement::checkMouseUpEvent(mouseX, mouseY, button, additionalElements);
}

bool TouchMovePad::checkHoverEvent(int mouseX,
                                   int mouseY,
                                   bmin::DynArray<UiElement*> additionalElements) {
  if (isDragging) {
    moveTo(mouseX - dragOffsetX, mouseY - dragOffsetY);
    return true;
  }
  return UiElement::checkHoverEvent(mouseX, mouseY, additionalElements);
}

void TouchMovePad::build() {
  children.clear();

  style.width = getContentWidth();
  style.height = getContentHeight();

  auto background = new OutsetRectangle(window, this);
  background->setId("background");
  background->setPos(style.x, getBodyY());
  background->setScale(style.scale);
  background->setProps(OutsetRectangleProps{
      .width = style.width,
      .height = getBodyHeight(),
      .color = Colors::BorderModalStandard,
      .colorTopRight = Colors::BorderModalStandardLight,
      .colorBottomLeft = Colors::BorderModalStandardDark,
      .borderSize = props.borderSize,
  });
  addChild(background);

  for (const auto& placement : buttonPlacements) {
    const auto [buttonX, buttonY] = getButtonPosition(placement.direction);

    auto button = new ButtonMove(window, this);
    button->setId(placement.id);
    button->setPos(buttonX, buttonY);
    button->setScale(style.scale);
    button->setProps(ButtonMoveProps{.direction = placement.direction});
    addChild(button);
  }

  auto dragBar = new OutsetRectangle(window, this);
  dragBar->setId("dragBar");
  dragBar->setPos(style.x, style.y);
  dragBar->setScale(style.scale);
  dragBar->setProps(OutsetRectangleProps{
      .width = style.width,
      .height = props.dragBarHeight,
      .color = Colors::BorderModalStandard,
      .colorTopRight = Colors::BorderModalStandardLight,
      .colorBottomLeft = Colors::BorderModalStandardDark,
      .borderSize = props.borderSize,
  });
  addChild(dragBar);

  auto dragHandle = new SpriteElement(window, this);
  dragHandle->setId("dragHandle");
  dragHandle->setPos(style.x + (style.width - borderButtonW) / 2,
                     style.y + (props.dragBarHeight - borderButtonH) / 2);
  dragHandle->setScale(style.scale);
  dragHandle->setProps(SpriteElementProps{
      .width = borderButtonW,
      .height = borderButtonH,
      .spriteName = "ui_border_buttons_0",
  });
  addChild(dragHandle);
}

void TouchMovePad::render(int dt) { UiElement::render(dt); }

} // namespace ui
