module;
#include <cstddef>
#include <cstdint>
#include <utility>
#if __has_include(<SDL.h>)
#include <SDL.h>
#include <SDL_pixels.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_pixels.h>
#endif

export module carcer.ui.components:PartyMemberIconSelector;
import carcer.actions.ui.UiSetCurrentPartyMember;
import carcer.actions.ui.UiSetCurrentPartyMemberInventory;
import carcer.actions.ui.UiSetCurrentPartyMemberMagic;
export import bmin.containers;
import bmin.string_interop;
export import carcer.ui.core;
import sdl2w;
import carcer.ui.elements;
#include "macros.h"

export {

// --- from ui/components/PartyMemberIconSelector.h ---
// IWYU pragma: keep

namespace ui {

enum class PartyMemberIconSelectorTarget { INVENTORY, PICKUP, MAGIC };

struct PartyMemberIconSelectorProps {
  bmin::DynArray<bmin::String> members;
  int selectedIndex = 0;
  PartyMemberIconSelectorTarget target = PartyMemberIconSelectorTarget::INVENTORY;
  int iconSize = 32;
  int iconGap = 4;

  SDL_Color spriteBgColor = Colors::LightGrey;
  SDL_Color spriteBgColorTopRight = Colors::White;
  SDL_Color spriteBgColorBottomLeft = Colors::ButtonModalGrey2;

  SDL_Color spriteSelectedBgColor = Colors::ButtonModalSelected;
  SDL_Color spriteSelectedBgColorTopRight = Colors::ButtonModalSelected;
  SDL_Color spriteSelectedBgColorBottomLeft = Colors::ButtonModalSelected;
};

class PartyMemberIconSelector : public UiElement {
private:
  PartyMemberIconSelectorProps props;

public:
  PartyMemberIconSelector(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~PartyMemberIconSelector() override = default;

  void setProps(const PartyMemberIconSelectorProps& _props);
  PartyMemberIconSelectorProps& getProps();
  const PartyMemberIconSelectorProps& getProps() const;

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

// --- from ui/observers/ObserverSetCurrentPartyMember.hpp ---
namespace ui {

class ObserverSetCurrentPartyMember : public ui::UiEventObserver,
                                      public state::StateManagerInterface {
  int partyMemberIndex;

public:
  explicit ObserverSetCurrentPartyMember(int _partyMemberIndex)
      : partyMemberIndex(_partyMemberIndex) {}

  void onClick(int mouseX, int mouseY, int button) override;
};

} // namespace ui

// --- from ui/observers/ObserverSetCurrentPartyMemberInventory.hpp ---
namespace ui {

class ObserverSetCurrentPartyMemberInventory : public ui::UiEventObserver,
                                               public state::StateManagerInterface {
  int partyMemberInventoryIndex;

public:
  explicit ObserverSetCurrentPartyMemberInventory(int _partyMemberInventoryIndex)
      : partyMemberInventoryIndex(_partyMemberInventoryIndex) {}

  void onClick(int mouseX, int mouseY, int button) override;
};

} // namespace ui

// --- from ui/observers/ObserverSetCurrentPartyMemberMagic.hpp ---
namespace ui {

class ObserverSetCurrentPartyMemberMagic : public ui::UiEventObserver,
                                           public state::StateManagerInterface {
  int partyMemberMagicIndex;

public:
  explicit ObserverSetCurrentPartyMemberMagic(int _partyMemberMagicIndex)
      : partyMemberMagicIndex(_partyMemberMagicIndex) {}

  void onClick(int mouseX, int mouseY, int button) override;
};

} // namespace ui

} // export

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
    groupProps.buttons.pushBack(ButtonGroupButtonProps{
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
  buttonGroup->setPos(style.x, style.y);
  buttonGroup->setScale(style.scale);
  buttonGroup->setProps(groupProps);

  for (size_t i = 0; i < props.members.size(); ++i) {
    if (props.target == PartyMemberIconSelectorTarget::PICKUP) {
      buttonGroup->addObserverToButtonAtIndex(static_cast<int>(i),
                                              new ObserverSetCurrentPartyMember(
                                                  static_cast<int>(i)));
    } else if (props.target == PartyMemberIconSelectorTarget::MAGIC) {
      buttonGroup->addObserverToButtonAtIndex(static_cast<int>(i),
                                              new ObserverSetCurrentPartyMemberMagic(
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

namespace ui {

void ObserverSetCurrentPartyMember::onClick(int mouseX, int mouseY, int button) {
    LOG(INFO) << "ObserverSetCurrentPartyMember::onClick index=" << partyMemberIndex
              << LOG_ENDL;
    auto stateManager = getStateManager();
    if (!stateManager) {
      return;
    }
    stateManager->enqueueAction(stateManager->getActionData(),
                                new state::actions::UiSetCurrentPartyMember(partyMemberIndex),
                                0);
  }

void ObserverSetCurrentPartyMemberInventory::onClick(int mouseX, int mouseY, int button) {
    LOG(INFO) << "ObserverSetCurrentPartyMemberInventory::onClick index="
              << partyMemberInventoryIndex << LOG_ENDL;
    auto stateManager = getStateManager();
    if (!stateManager) {
      return;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(),
        new state::actions::UiSetCurrentPartyMemberInventory(partyMemberInventoryIndex),
        0);
  }

void ObserverSetCurrentPartyMemberMagic::onClick(int mouseX, int mouseY, int button) {
    LOG(INFO) << "ObserverSetCurrentPartyMemberMagic::onClick index="
              << partyMemberMagicIndex << LOG_ENDL;
    auto stateManager = getStateManager();
    if (!stateManager) {
      return;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(),
        new state::actions::UiSetCurrentPartyMemberMagic(partyMemberMagicIndex),
        0);
  }

} // namespace ui
