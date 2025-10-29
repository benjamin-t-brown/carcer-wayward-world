#include "ButtonWorldAction.h"
#include "lib/sdl2w/L10n.h"
#include "types/WorldActions.h"
#include "ui/elements/SpriteElement.h"
#include <memory>
#include <sstream>

namespace ui {

using namespace types;

class ButtonWorldActionDefaultObserver : public UiEventObserver {
  ButtonWorldAction* buttonWorldAction;

public:
  ButtonWorldActionDefaultObserver(ButtonWorldAction* _buttonWorldAction)
      : buttonWorldAction(_buttonWorldAction) {}
  ~ButtonWorldActionDefaultObserver() override = default;
  void onMouseDown(int x, int y, int button) override {
    buttonWorldAction->isActive = true;
  }
  void onMouseUp(int x, int y, int button) override {
    buttonWorldAction->isActive = false;
  }
};

ButtonWorldAction::ButtonWorldAction(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  addEventObserver(std::make_unique<ButtonWorldActionDefaultObserver>(this));
  style.textAlign = TextAlign::CENTER;
  style.fontSize = sdl2w::TEXT_SIZE_20;
  shouldPropagateEventsToChildren = false;

  // Set default size
  style.width = 32;
  style.height = 32;
}

ButtonWorldActionMapping
ButtonWorldAction::getButtonWorldActionMapping(types::WorldActionType worldActionType) {
  switch (worldActionType) {
  case WorldActionType::JUMP:
    return ButtonWorldActionMapping{TRANSLATE("Jump"), 17, true};
    break;
  case WorldActionType::ABILITY:
    return ButtonWorldActionMapping{TRANSLATE("Ability"), 1};
    break;
  // case WorldActionType::STATUS:
  //   return ButtonWorldActionMapping{TRANSLATE("Status"), 2};
  //   break;
  case WorldActionType::TALK:
    return ButtonWorldActionMapping{TRANSLATE("Talk"), 3};
    break;
  case WorldActionType::END_FIGHT:
    return ButtonWorldActionMapping{TRANSLATE("End Fight"), 4};
    break;
  case WorldActionType::GET:
    return ButtonWorldActionMapping{TRANSLATE("Get"), 16, true};
    break;
  // case WorldActionType::MAP:
  //   return ButtonWorldActionMapping{TRANSLATE("Map"), 6};
  //   break;
  case WorldActionType::SNEAK:
    return ButtonWorldActionMapping{TRANSLATE("Sneak"), 7};
    break;
  case WorldActionType::START_FIGHT:
    return ButtonWorldActionMapping{TRANSLATE("Start Fight"), 8};
    break;
  case WorldActionType::UNLOCK:
    return ButtonWorldActionMapping{TRANSLATE("Unlock"), 9};
    break;
  case WorldActionType::EXAMINE:
    return ButtonWorldActionMapping{TRANSLATE("Examine"), 10};
    break;
  // case WorldActionType::INVENTORY:
  //   return ButtonWorldActionMapping{TRANSLATE("Inventory"), 11};
  //   break;
  case WorldActionType::SHOOT:
    return ButtonWorldActionMapping{TRANSLATE("Shoot"), 0, true};
    break;
  case WorldActionType::DEFEND:
    return ButtonWorldActionMapping{TRANSLATE("Defend"), 1, true};
    break;
  case WorldActionType::INTERACT:
    return ButtonWorldActionMapping{TRANSLATE("Interact"), 2, true};
    break;
  case WorldActionType::REST:
    return ButtonWorldActionMapping{TRANSLATE("Rest"), 3, true};
    break;
  case WorldActionType::JOURNAL:
    return ButtonWorldActionMapping{TRANSLATE("Journal"), 4, true};
    break;
  case WorldActionType::STATUS:
    return ButtonWorldActionMapping{TRANSLATE("Status"), 5, true};
    break;
  case WorldActionType::INVENTORY:
    return ButtonWorldActionMapping{TRANSLATE("Inventory"), 6, true};
    break;
  case WorldActionType::MAP:
    return ButtonWorldActionMapping{TRANSLATE("Map"), 7, true};
    break;
  case WorldActionType::MAP_OUTDOOR:
    return ButtonWorldActionMapping{TRANSLATE("Map"), 6};
    break;
  }
  return ButtonWorldActionMapping{TRANSLATE("Examine"), 0};
}

bool ButtonWorldAction::checkIfWorldActionButtonIsSmall(
    types::WorldActionType worldActionType) {
  auto mapping = getButtonWorldActionMapping(worldActionType);
  return mapping.isSmall;
}

void ButtonWorldAction::setProps(const ButtonWorldActionProps& _props) {
  props = _props;
  build();
}

ButtonWorldActionProps& ButtonWorldAction::getProps() { return props; }

const ButtonWorldActionProps& ButtonWorldAction::getProps() const { return props; }

void ButtonWorldAction::build() {
  children.clear();

  auto mapping = getButtonWorldActionMapping(props.worldActionType);

  auto spriteElement = std::make_unique<SpriteElement>(window);
  BaseStyle spriteStyle;
  spriteStyle.x = style.x;
  spriteStyle.y = style.y;
  spriteStyle.scale = style.scale;
  spriteElement->setStyle(spriteStyle);

  int startingSpriteIndex =
      mapping.isSmall ? smallStartingSpriteIndex : normalStartingSpriteIndex;
  std::stringstream ss;
  ss << spriteSheetName << "_";
  if (mapping.isSmall) {
    ss << "half_";
  }

  if (isActive) {
    if (mapping.isSmall) {
      startingSpriteIndex += smallSpriteOffsetToActive;
    } else {
      startingSpriteIndex += normalSpriteOffsetToActive;
    }
  }
  ss << (startingSpriteIndex + mapping.spriteIndex);

  if (mapping.isSmall) {
    spriteStyle.width = 32;
    spriteStyle.height = 16;
  } else {
    spriteStyle.width = 32;
    spriteStyle.height = 32;
  }

  style.width = spriteStyle.width;
  style.height = spriteStyle.height;

  std::string spriteName = ss.str();
  spriteElement->setSprite(spriteName);
  children.push_back(std::move(spriteElement));
}

void ButtonWorldAction::render(int dt) {
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
