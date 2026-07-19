#include "ui/helpers/worldActions.h"

#include "state/StateManager.h"
#include "state/actions/ui/heldMove/UiUpdateHeldMove.hpp"
#include "state/actions/world/WorldSetActionMode.hpp"

namespace ui {

std::optional<state::WorldActionType>
getWorldActionFromKeyboardShortcut(std::string_view key) {
  if (key == "l" || key == "L") {
    return state::WorldActionType::EXAMINE;
  }
  if (key == "t" || key == "T") {
    return state::WorldActionType::TALK;
  }
  return std::nullopt;
}

void setHeldMoveActive(state::StateManager& stateManager, bool isActive) {
  auto nextHeldMove = stateManager.getState().uiState.heldMove;
  nextHeldMove.isActive = isActive;
  stateManager.pllAction(stateManager.getActionData(),
                         new state::actions::UiUpdateHeldMove(nextHeldMove),
                         0);
}

void cancelCurrentWorldActionMode(state::StateManager& stateManager) {
  if (stateManager.getState().world.actionMode == model::WorldActionMode::NONE) {
    return;
  }
  setHeldMoveActive(stateManager, false);
  stateManager.pllAction(
      stateManager.getActionData(),
      new state::actions::WorldSetActionMode(model::WorldActionMode::NONE),
      0);
}

void activateWorldAction(state::StateManager& stateManager,
                         state::WorldActionType worldActionType) {

  const auto currentMode = stateManager.getState().world.actionMode;

  switch (worldActionType) {
  case state::WorldActionType::EXAMINE:
    setHeldMoveActive(stateManager, false);
    if (currentMode == model::WorldActionMode::EXAMINE) {
      cancelCurrentWorldActionMode(stateManager);
      break;
    }
    stateManager.pllAction(
        stateManager.getActionData(),
        new state::actions::WorldSetActionMode(model::WorldActionMode::EXAMINE),
        0);
    break;
  case state::WorldActionType::TALK:
    setHeldMoveActive(stateManager, false);
    if (currentMode == model::WorldActionMode::TALK) {
      cancelCurrentWorldActionMode(stateManager);
      break;
    }
    stateManager.pllAction(
        stateManager.getActionData(),
        new state::actions::WorldSetActionMode(model::WorldActionMode::TALK),
        0);
    break;
  default:
    break;
  }
}

} // namespace ui
