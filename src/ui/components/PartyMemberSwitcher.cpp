#include "PartyMemberSwitcher.h"
#include "ui/elements/OutsetRectangle.h"
#include "ui/elements/Quad.h"
#include "ui/elements/buttons/ButtonModal.h"
#include "ui/observers/ObserverUpdateCurrentPartyMember.hpp"
#include <algorithm>

namespace ui {

PartyMemberSwitcher::PartyMemberSwitcher(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

void PartyMemberSwitcher::setProps(const PartyMemberSwitcherProps& _props) {
  props = _props;
  build();
}

PartyMemberSwitcherProps& PartyMemberSwitcher::getProps() { return props; }

const PartyMemberSwitcherProps& PartyMemberSwitcher::getProps() const { return props; }

const std::pair<int, int> PartyMemberSwitcher::getDims() const {
  const int w = props.buttonSize + props.buttonSpacing + props.spriteBoxSize +
                props.buttonSpacing + props.buttonSize;
  const int h = std::max(props.buttonSize, props.spriteBoxSize);
  return {static_cast<int>(w * style.scale), static_cast<int>(h * style.scale)};
}

void PartyMemberSwitcher::build() {
  children.clear();

  const int buttonSize = props.buttonSize;
  const int spriteSize = props.spriteBoxSize;
  const int spacing = props.buttonSpacing;
  const int totalWidth = buttonSize + spacing + spriteSize + spacing + buttonSize;
  const int totalHeight = std::max(buttonSize, spriteSize);

  style.width = totalWidth;
  style.height = totalHeight;

  auto container = new Quad(window, this);
  container->setPos(style.x, style.y);
  container->setScale(style.scale);
  container->setProps(QuadProps{
      .width = totalWidth,
      .height = totalHeight,
  });
  addChild(container);

  auto leftBtn = new ButtonModal(window, container);
  leftBtn->setId("prevPartyMember");
  leftBtn->setPos(0, (totalHeight - buttonSize) / 2);
  leftBtn->setProps(ButtonModalProps{.text = "<", .width = buttonSize, .height = buttonSize});
  leftBtn->addEventObserver(
      new ObserverUpdateCurrentPartyMember(props.partyMemberIndex, -1));
  container->addChild(leftBtn);

  const int spriteX = buttonSize + spacing;

  auto spriteRect = new OutsetRectangle(window, container);
  spriteRect->setPos(spriteX, 0);
  spriteRect->setProps(OutsetRectangleProps{
      .width = spriteSize,
      .height = spriteSize,
      .color = props.spriteBgColor,
      .colorTopRight = props.spriteBorderColor1,
      .colorBottomLeft = props.spriteBorderColor2,
      .borderSize = props.spriteBorderSize,
  });
  container->addChild(spriteRect);

  auto sprite = new Quad(window, container);
  sprite->setPos(spriteX + 1, 1);
  sprite->setProps(QuadProps{
      .width = spriteSize,
      .height = spriteSize,
      .bgSprite = props.spriteName,
  });
  container->addChild(sprite);

  auto rightBtn = new ButtonModal(window, container);
  rightBtn->setId("nextPartyMember");
  rightBtn->setPos(spriteX + spriteSize + spacing, (totalHeight - buttonSize) / 2);
  rightBtn->setProps(ButtonModalProps{.text = ">", .width = buttonSize, .height = buttonSize});
  rightBtn->addEventObserver(
      new ObserverUpdateCurrentPartyMember(props.partyMemberIndex, 1));
  container->addChild(rightBtn);
}

void PartyMemberSwitcher::render(int dt) { UiElement::render(dt); }

} // namespace ui
