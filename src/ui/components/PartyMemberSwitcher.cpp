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
  auto& containerStyle = container->getStyle();
  containerStyle.x = style.x;
  containerStyle.y = style.y;
  containerStyle.width = totalWidth;
  containerStyle.height = totalHeight;
  containerStyle.scale = style.scale;
  container->setProps(QuadProps{});
  addChild(container);

  auto leftBtn = new ButtonModal(window, container);
  leftBtn->setId("prevPartyMember");
  auto& leftStyle = leftBtn->getStyle();
  leftStyle.x = 0;
  leftStyle.y = (totalHeight - buttonSize) / 2;
  leftStyle.width = buttonSize;
  leftStyle.height = buttonSize;
  leftBtn->setProps(ButtonModalProps{.text = "<"});
  leftBtn->addEventObserver(
      new ObserverUpdateCurrentPartyMember(props.partyMemberIndex, -1));
  container->addChild(leftBtn);

  const int spriteX = buttonSize + spacing;

  auto spriteRect = new OutsetRectangle(window, container);
  auto& spriteRectStyle = spriteRect->getStyle();
  spriteRectStyle.x = spriteX;
  spriteRectStyle.y = 0;
  spriteRectStyle.width = spriteSize;
  spriteRectStyle.height = spriteSize;
  spriteRect->setProps(OutsetRectangleProps{
      .color = props.spriteBgColor,
      .colorTopRight = props.spriteBorderColor1,
      .colorBottomLeft = props.spriteBorderColor2,
      .borderSize = props.spriteBorderSize,
  });
  container->addChild(spriteRect);

  auto sprite = new Quad(window, container);
  auto& spriteStyle = sprite->getStyle();
  spriteStyle.x = spriteX + 1;
  spriteStyle.y = 1;
  spriteStyle.width = spriteSize;
  spriteStyle.height = spriteSize;
  sprite->setProps(QuadProps{
      .bgSprite = props.spriteName,
  });
  container->addChild(sprite);

  auto rightBtn = new ButtonModal(window, container);
  rightBtn->setId("nextPartyMember");
  auto& rightStyle = rightBtn->getStyle();
  rightStyle.x = spriteX + spriteSize + spacing;
  rightStyle.y = (totalHeight - buttonSize) / 2;
  rightStyle.width = buttonSize;
  rightStyle.height = buttonSize;
  rightBtn->setProps(ButtonModalProps{.text = ">"});
  rightBtn->addEventObserver(
      new ObserverUpdateCurrentPartyMember(props.partyMemberIndex, 1));
  container->addChild(rightBtn);
}

void PartyMemberSwitcher::render(int dt) { UiElement::render(dt); }

} // namespace ui
