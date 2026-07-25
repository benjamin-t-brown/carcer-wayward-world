#include "ui/helpers/worldActions.h"

#include "state/StateManager.h"
#include "state/actions/ui/UiShowLayerInventory.hpp"
#include "state/actions/ui/heldMove/UiUpdateHeldMove.hpp"
#include "state/actions/world/WorldInteractAt.hpp"
#include "state/actions/world/WorldSetActionMode.hpp"

namespace ui {

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
                         state::WorldActionType worldActionType,
                         sdl2w::Window* window) {

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
  case state::WorldActionType::INVENTORY:
    if (!window) {
      break;
    }
    setHeldMoveActive(stateManager, false);
    stateManager.enqueueAction(stateManager.getActionData(),
                               new state::actions::UiShowLayerInventory(window),
                               0);
    break;
  case state::WorldActionType::INTERACT:
    setHeldMoveActive(stateManager, false);
    cancelCurrentWorldActionMode(stateManager);
    stateManager.enqueueAction(stateManager.getActionData(),
                               new state::actions::WorldInteractAt(),
                               0);
    break;
  default:
    break;
  }
}

} // namespace ui
