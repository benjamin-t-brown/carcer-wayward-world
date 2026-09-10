#include "WorldInputController.h"
#include "bmin/StringInterop.h"
#include "layers/UiLayer.h"
#include "model/Combat.h"
#include "state/StateManager.h"
#include "actions/navigation/heldMove/UiUpdateHeldMove.hpp"
#include "actions/world/WorldMoveActionAim.hpp"
#include "actions/world/WorldSetActionAim.hpp"
#include "ui/components/MapView.h"
#include "ui/helpers/keyboardShortcuts.h"
#include "ui/helpers/worldActions.h"
#include "ui/helpers/worldCommands.h"

namespace layers {

void WorldInputController::onKeyDown(std::string_view key, int /*keyCode*/) {
  if (owner.getState() != LayerState::ON) {
    return;
  }
  auto stateManager = getStateManager();
  if (!stateManager) {
    return;
  }
  auto& world = stateManager->getState().world;

  if (ui::isCancelActionKey(key)) {
    bmin::List<model::WorldActionMode> cancellableActionModes = {
        model::WorldActionMode::EXAMINE,
        model::WorldActionMode::TALK,
        model::WorldActionMode::SPELL,
    };
    if (cancellableActionModes.contains(world.actionMode)) {
      ui::cancelCurrentWorldActionMode(*stateManager);
      return;
    }
  }

  if (ui::isCombatWaitKey(key) && world.combat.active) {
    ui::setHeldMoveActive(*stateManager, false);
    ui::enqueueCombatWait(*stateManager);
    return;
  }

  // Block world-action shortcuts / aim confirm while town AI is resolving.
  if (!world.resolvingTownEnemyAi) {
    // `r` always opens magic setup (not remapped through combat Ability → cast).
    if (ui::isOpenMagicSetupKey(key)) {
      ui::showMagicSetupLayer(*stateManager, owner.getWindow());
      return;
    }
    if (ui::isOpenSpellCastKey(key)) {
      ui::showSpellCastLayer(*stateManager, owner.getWindow());
      return;
    }
    if (auto actionType = ui::getWorldActionFromKeyboardShortcut(
            key, stateManager->getState().turnMode)) {
      ui::activateWorldAction(*stateManager, *actionType, owner.getWindow());
      return;
    }
  }

  const bool isAimMode = world.actionMode == model::WorldActionMode::EXAMINE ||
                         world.actionMode == model::WorldActionMode::TALK ||
                         world.actionMode == model::WorldActionMode::SPELL;

  if (isAimMode && ui::isConfirmActionKey(key)) {
    if (!world.resolvingTownEnemyAi && world.actionAimTile) {
      ui::confirmWorldActionAim(*stateManager, owner.getWindow(), getDatabase(),
                                world.actionAimTile->x, world.actionAimTile->y);
    }
    return;
  }

  auto moveDelta = ui::getMoveDeltaForKey(key);
  if (!moveDelta) {
    return;
  }

  if (isAimMode) {
    if (world.resolvingTownEnemyAi) {
      return;
    }
    ui::setHeldMoveActive(*stateManager, false);
    stateManager->enqueueAction(state::makeAction<state::actions::WorldMoveActionAim>(moveDelta->dx, moveDelta->dy),
        0);
    return;
  }

  const auto& heldMove = stateManager->getState().uiState.heldMove;
  // Ignore OS/SDL key-repeat events for the same held key; we time repeats ourselves.
  if (heldMove.isActive && heldMove.key.sliceView() == key) {
    return;
  }

  const bool canEnqueueMove =
      !world.resolvingTownEnemyAi &&
      (!world.combat.active || ui::canPlayerIssueCombatMove(stateManager->getState()));

  state::HeldMove nextHeldMove{
      .isActive = true,
      .key = bmin::fromStringView(key),
      .dx = moveDelta->dx,
      .dy = moveDelta->dy,
  };
  model::timerStructRestart(nextHeldMove.initialDelay);
  model::timerStructRestart(nextHeldMove.moveDelay);
  stateManager->parallelAction(state::makeAction<state::actions::UiUpdateHeldMove>(nextHeldMove),
                          0);
  if (canEnqueueMove) {
    ui::enqueueMapMove(*stateManager, moveDelta->dx, moveDelta->dy);
  }
}

void WorldInputController::onKeyUp(std::string_view key, int /*keyCode*/) {
  auto stateManager = getStateManager();
  if (!stateManager) {
    return;
  }
  const auto& heldMove = stateManager->getState().uiState.heldMove;
  if (heldMove.isActive && heldMove.key.sliceView() == key) {
    ui::setHeldMoveActive(*stateManager, false);
  }
}

void WorldInputController::updateAimFromMouse(int x, int y) {
  if (owner.getState() != LayerState::ON) {
    return;
  }
  // Only react to actual mouse movement so keyboard aim isn't fought by a
  // stationary cursor still sitting over the map.
  if (hasLastMousePos && x == lastMouseX && y == lastMouseY) {
    return;
  }
  hasLastMousePos = true;
  lastMouseX = x;
  lastMouseY = y;

  auto* stateManager = getStateManager();
  if (!stateManager) {
    return;
  }
  const auto actionMode = stateManager->getState().world.actionMode;
  if (actionMode != model::WorldActionMode::EXAMINE &&
      actionMode != model::WorldActionMode::TALK &&
      actionMode != model::WorldActionMode::SPELL) {
    return;
  }
  auto* mapView = owner.getUiElement<ui::MapView>("mapView");
  if (!mapView) {
    return;
  }
  auto tile = mapView->screenToTile(x, y);
  if (!tile) {
    return;
  }
  const auto& aim = stateManager->getState().world.actionAimTile;
  if (aim && aim->x == tile->x && aim->y == tile->y) {
    return;
  }
  // Parallel so hover stays responsive even if sequential actions are waiting.
  stateManager->parallelAction(state::makeAction<state::actions::WorldSetActionAim>(tile->x, tile->y),
                          0);
}

bool WorldInputController::onMouseDown(int x, int y, int button) {
  // SDL_BUTTON_LEFT == 1
  if (owner.getState() == LayerState::ON && button == 1) {
    auto* stateManager = getStateManager();
    if (stateManager) {
      const auto actionMode = stateManager->getState().world.actionMode;
      if (actionMode == model::WorldActionMode::EXAMINE ||
          actionMode == model::WorldActionMode::TALK ||
          actionMode == model::WorldActionMode::SPELL) {
        if (auto* mapView = owner.getUiElement<ui::MapView>("mapView")) {
          if (auto tile = mapView->screenToTile(x, y)) {
            stateManager->enqueueAction(state::makeAction<state::actions::WorldSetActionAim>(tile->x, tile->y),
                0);
            ui::confirmWorldActionAim(*stateManager, owner.getWindow(), getDatabase(),
                                      tile->x, tile->y);
            return true;
          }
        }
      }
    }
  }
  return false;
}

void WorldInputController::updateHeldMoveRepeat(int deltaTime) {
  auto stateManager = getStateManager();
  if (!stateManager) {
    return;
  }

  auto& heldMove = stateManager->getState().uiState.heldMove;
  if (!heldMove.isActive) {
    return;
  }

  if (!owner.getWindow()->getEvents().isKeyPressed(heldMove.key.sliceView())) {
    ui::setHeldMoveActive(*stateManager, false);
    return;
  }

  if (stateManager->getState().world.actionMode != model::WorldActionMode::NONE) {
    ui::setHeldMoveActive(*stateManager, false);
    return;
  }

  // Pause while town AI or combat cannot accept a move; keep hold active so repeats
  // resume.
  if (stateManager->getState().world.resolvingTownEnemyAi) {
    return;
  }

  if (stateManager->getState().world.combat.active &&
      !ui::canPlayerIssueCombatMove(stateManager->getState())) {
    return;
  }

  model::timerStructUpdate(heldMove.initialDelay, deltaTime);
  model::timerStructUpdate(heldMove.moveDelay, deltaTime);

  if (model::timerStructIsComplete(heldMove.initialDelay) &&
      model::timerStructIsComplete(heldMove.moveDelay)) {
    model::timerStructRestart(heldMove.moveDelay);
    ui::enqueueMapMove(*stateManager, heldMove.dx, heldMove.dy);
  }
}

} // namespace layers
