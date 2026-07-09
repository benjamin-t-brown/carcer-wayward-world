#include "ButtonSprite.h"
#include "ui/elements/OutsetRectangle.h"
#include "ui/elements/Quad.h"

namespace ui {

class ButtonSpriteDefaultObserver : public UiEventObserver {
  ButtonSprite* buttonSprite;

public:
  ButtonSpriteDefaultObserver(ButtonSprite* _buttonSprite) : buttonSprite(_buttonSprite) {}
  ~ButtonSpriteDefaultObserver() override = default;
  void onMouseDown(int x, int y, int button) override { buttonSprite->isActive = true; }
  void onMouseUp(int x, int y, int button) override { buttonSprite->isActive = false; }
};

int ButtonSprite::getLogicalWidth() const {
  return props.spriteWidth + props.padding * 2;
}

int ButtonSprite::getLogicalHeight() const {
  return props.spriteHeight + props.padding * 2;
}

ButtonSprite::ButtonSprite(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  addEventObserver(new ButtonSpriteDefaultObserver(this));
  shouldPropagateEventsToChildren = false;
}

void ButtonSprite::setProps(const ButtonSpriteProps& _props) {
  props = _props;
  build();
}

ButtonSpriteProps& ButtonSprite::getProps() { return props; }

const ButtonSpriteProps& ButtonSprite::getProps() const { return props; }

const std::pair<int, int> ButtonSprite::getDims() const {
  return {static_cast<int>(getLogicalWidth() * style.scale),
          static_cast<int>(getLogicalHeight() * style.scale)};
}

void ButtonSprite::build() {
  children.clear();

  style.width = getLogicalWidth();
  style.height = getLogicalHeight();

  const int pressOffset = (isInActiveMode && !props.isSelected) ? 1 : 0;

  auto rect = new OutsetRectangle(window);
  rect->setPos(style.x, style.y);
  rect->setScale(style.scale);

  OutsetRectangleProps rectProps;
  rectProps.width = style.width;
  rectProps.height = style.height;
  if (props.isSelected) {
    rectProps.borderSize = isInActiveMode ? 0 : props.selectedBorderSize;
    rectProps.color = props.selectedBgColor;
    rectProps.colorTopRight = props.selectedBgColorTopRight;
    rectProps.colorBottomLeft = props.selectedBgColorBottomLeft;
  } else {
    rectProps.borderSize = isInActiveMode ? 0 : props.borderSize;
    rectProps.color = props.bgColor;
    rectProps.colorTopRight = props.bgColorTopRight;
    rectProps.colorBottomLeft = props.bgColorBottomLeft;
  }
  rect->setProps(rectProps);
  addChild(rect);

  if (!props.spriteName.empty()) {
    auto sprite = new Quad(window, this);
    sprite->setPos(style.x + static_cast<int>((props.padding + pressOffset) * style.scale),
                   style.y + static_cast<int>((props.padding + pressOffset) * style.scale));
    sprite->setScale(style.scale);
    sprite->setProps(QuadProps{
        .width = props.spriteWidth,
        .height = props.spriteHeight,
        .bgSprite = props.spriteName,
    });
    addChild(sprite);
  }
}

void ButtonSprite::render(int dt) {
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

  UiElement::render(dt);
}

} // namespace ui
