#include "ui/helpers/keyboardShortcuts.h"

namespace ui {

std::optional<state::WorldActionType>
getWorldActionFromKeyboardShortcut(std::string_view key, model::TurnMode turnMode) {
  if (key == "f" || key == "F") {
    if (turnMode == model::TurnMode::TURN_COMBAT) {
      return state::WorldActionType::END_FIGHT;
    }
    if (turnMode == model::TurnMode::TURN_TOWN) {
      return state::WorldActionType::START_FIGHT;
    }
    return std::nullopt;
  }
  if ((key == "e" || key == "E") && turnMode == model::TurnMode::TURN_COMBAT) {
    return state::WorldActionType::END_FIGHT;
  }
  if (key == "l" || key == "L") {
    return state::WorldActionType::EXAMINE;
  }
  if (key == "t" || key == "T") {
    return state::WorldActionType::TALK;
  }
  if (key == "i" || key == "I") {
    return state::WorldActionType::INVENTORY;
  }
  if (key == " ") {
    return state::WorldActionType::INTERACT;
  }
  if (key == "Keypad 5" && turnMode != model::TurnMode::TURN_COMBAT) {
    return state::WorldActionType::INTERACT;
  }
  return std::nullopt;
}

bool isCombatWaitKey(std::string_view key) {
  return key == "Keypad 5";
}

std::optional<MoveDelta> getMoveDeltaForKey(std::string_view key) {
  if (key == "Up" || key == "Keypad 8") {
    return MoveDelta{0, -1};
  }
  if (key == "Down" || key == "Keypad 2") {
    return MoveDelta{0, 1};
  }
  if (key == "Left" || key == "Keypad 4") {
    return MoveDelta{-1, 0};
  }
  if (key == "Right" || key == "Keypad 6") {
    return MoveDelta{1, 0};
  }
  if (key == "Keypad 7") {
    return MoveDelta{-1, -1};
  }
  if (key == "Keypad 9") {
    return MoveDelta{1, -1};
  }
  if (key == "Keypad 1") {
    return MoveDelta{-1, 1};
  }
  if (key == "Keypad 3") {
    return MoveDelta{1, 1};
  }
  return std::nullopt;
}

bool isCancelActionKey(std::string_view key) { return key == "Escape"; }

bool isConfirmActionKey(std::string_view key) {
  return key == "Return" || key == "Keypad Enter";
}

} // namespace ui
