#include "PartyMemberIconSelector.h"
#include "ui/elements/buttons/ButtonGroup.h"
#include "ui/observers/ObserverSetCurrentPartyMember.hpp"
#include "ui/observers/ObserverSetCurrentPartyMemberInventory.hpp"

namespace ui {

PartyMemberIconSelector::PartyMemberIconSelector(sdl2w::Window* _window,
                                                 UiElement* _parent)
    : UiElement(_window, _parent) {}

void PartyMemberIconSelector::setProps(const PartyMemberIconSelectorProps& _props) {
  props = _props;
  build();
}

PartyMemberIconSelectorProps& PartyMemberIconSelector::getProps() { return props; }

const PartyMemberIconSelectorProps& PartyMemberIconSelector::getProps() const {
  return props;
}

const std::pair<int, int> PartyMemberIconSelector::getDims() const {
  if (children.empty()) {
    if (props.members.empty()) {
      return {0, static_cast<int>(props.iconSize * style.scale)};
    }
    const int memberCount = static_cast<int>(props.members.size());
    const int w = memberCount * props.iconSize + (memberCount - 1) * props.iconGap;
    return {static_cast<int>(w * style.scale), static_cast<int>(props.iconSize * style.scale)};
  }
  return children[0]->getDims();
}

void PartyMemberIconSelector::build() {
  children.clear();

  if (props.members.empty()) {
    return;
  }

  ButtonGroupProps groupProps;
  groupProps.alignment = ButtonGroupAlignment::LEFT;
  groupProps.padding = 0;
  groupProps.buttonSpacing = props.iconGap;
  groupProps.buttonWidth = props.iconSize;
  groupProps.buttonHeight = props.iconSize;
  groupProps.spriteBgColor = props.spriteBgColor;
  groupProps.spriteBgColorTopRight = props.spriteBgColorTopRight;
  groupProps.spriteBgColorBottomLeft = props.spriteBgColorBottomLeft;
  groupProps.spriteSelectedBgColor = props.spriteSelectedBgColor;
  groupProps.spriteSelectedBgColorTopRight = props.spriteSelectedBgColorTopRight;
  groupProps.spriteSelectedBgColorBottomLeft = props.spriteSelectedBgColorBottomLeft;

  for (size_t i = 0; i < props.members.size(); ++i) {
    groupProps.buttons.push_back(ButtonGroupButtonProps{
        .type = ButtonGroupButtonType::SPRITE,
        .spriteName = props.members[i],
        .spriteWidth = props.iconSize,
        .spriteHeight = props.iconSize,
        .spritePadding = 0,
        .isSelected = static_cast<int>(i) == props.selectedIndex,
    });
  }

  auto buttonGroup = new ButtonGroup(window, this);
  buttonGroup->setId("partyMemberButtonGroup");
  auto& groupStyle = buttonGroup->getStyle();
  groupStyle.x = style.x;
  groupStyle.y = style.y;
  groupStyle.scale = style.scale;
  buttonGroup->setProps(groupProps);

  for (size_t i = 0; i < props.members.size(); ++i) {
    if (props.target == PartyMemberIconSelectorTarget::PICKUP) {
      buttonGroup->addObserverToButtonAtIndex(static_cast<int>(i),
                                              new ObserverSetCurrentPartyMember(
                                                  static_cast<int>(i)));
    } else {
      buttonGroup->addObserverToButtonAtIndex(static_cast<int>(i),
                                              new ObserverSetCurrentPartyMemberInventory(
                                                  static_cast<int>(i)));
    }
  }

  addChild(buttonGroup);
}

void PartyMemberIconSelector::render(int dt) { UiElement::render(dt); }

} // namespace ui
