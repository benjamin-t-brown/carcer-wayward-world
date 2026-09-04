module;
#include <cstddef>
#include <cstdint>
#include <utility>

export module carcer.ui.elements:ButtonWorldAction;
export import bmin.containers;
import bmin.string_interop;
export import carcer.state;
export import carcer.ui.core;
import sdl2w;
import :SpriteElement;
#include "macros.h"

export {

// --- from ui/elements/buttons/ButtonWorldAction.h ---
namespace ui {

// ButtonWorldAction-specific properties
struct ButtonWorldActionProps {
  state::WorldActionType worldActionType;
};

struct ButtonWorldActionMapping {
  bmin::String label;
  int spriteIndex;
  bool isSmall = false;
};

// ButtonWorldAction element - renders a clickable button typically used to interact with
// the world Uses Position, Size, Scale from BaseStyle
class ButtonWorldAction : public UiElement {
private:
  ButtonWorldActionProps props;
  bool isInHoverMode = false;
  bool isInActiveMode = false;
  const bmin::String spriteSheetName = "ui_action_buttons";
  const int normalStartingSpriteIndex = 16;
  const int smallStartingSpriteIndex = 0;
  const int normalSpriteOffsetToActive = 16;
  const int smallSpriteOffsetToActive = 8;

  static ButtonWorldActionMapping
  getButtonWorldActionMapping(state::WorldActionType worldActionType);

public:
  bool isActive = false;
  bool isModeSelected = false;
  ButtonWorldAction(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ButtonWorldAction() override = default;

  static bool checkIfWorldActionButtonIsSmall(state::WorldActionType worldActionType);

  void setProps(const ButtonWorldActionProps& _props);
  ButtonWorldActionProps& getProps();
  const ButtonWorldActionProps& getProps() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

namespace ui {

using state::WorldActionType;

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
  addEventObserver(new ButtonWorldActionDefaultObserver(this));
  shouldPropagateEventsToChildren = false;

  style.width = 32;
  style.height = 32;
}

ButtonWorldActionMapping
ButtonWorldAction::getButtonWorldActionMapping(state::WorldActionType worldActionType) {
  switch (worldActionType) {
  case WorldActionType::JUMP:
    return ButtonWorldActionMapping{TRANSLATE("Jump"), 17, true};
    break;
  case WorldActionType::ABILITY:
    return ButtonWorldActionMapping{TRANSLATE("Ability"), 1};
    break;
  case WorldActionType::TALK:
    return ButtonWorldActionMapping{TRANSLATE("Talk"), 3};
    break;
  case WorldActionType::END_FIGHT:
    return ButtonWorldActionMapping{TRANSLATE("End Fight"), 4};
    break;
  case WorldActionType::GET:
    return ButtonWorldActionMapping{TRANSLATE("Get"), 16, true};
    break;
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
    state::WorldActionType worldActionType) {
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

  int spriteW = mapping.isSmall ? 32 : 32;
  int spriteH = mapping.isSmall ? 16 : 32;
  style.width = spriteW;
  style.height = spriteH;

  int startingSpriteIndex =
      mapping.isSmall ? smallStartingSpriteIndex : normalStartingSpriteIndex;
  bmin::StringStream ss;
  ss << spriteSheetName << "_";
  if (mapping.isSmall) {
    ss << "half_";
  }

  if (isActive || isModeSelected) {
    if (mapping.isSmall) {
      startingSpriteIndex += smallSpriteOffsetToActive;
    } else {
      startingSpriteIndex += normalSpriteOffsetToActive;
    }
  }
  ss << (startingSpriteIndex + mapping.spriteIndex);

  auto spriteElement = bmin::makeUnique<SpriteElement>(window);
  spriteElement->setPos(style.x, style.y);
  spriteElement->setScale(style.scale);
  spriteElement->setProps(SpriteElementProps{
      .width = spriteW,
      .height = spriteH,
      .spriteName = bmin::String(ss.str().cStr()),
  });
  children.pushBack(bmin::UniquePtr<UiElement>(spriteElement.release()));
}

void ButtonWorldAction::render(int dt) {
  const bool showActiveSprite = isActive || isModeSelected;
  if (showActiveSprite) {
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
