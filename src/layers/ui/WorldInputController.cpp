#include "WorldInputController.h"
#include "bmin/StringInterop.h"
#include "layers/UiLayer.h"
#include "model/Combat.h"
#include "state/StateManager.h"
#include "actions/navigation/heldMove/UiUpdateHeldMove.hpp"
#include "actions/world/WorldMoveActionAim.hpp"
#include "actions/world/WorldSetActionAim.hpp"
#include "ui/components/MapView.h"
#include "ui/elements/buttons/ButtonWorldAction.h"
#include "ui/helpers/keyboardShortcuts.h"
#include "ui/helpers/uiSounds.h"
#include "ui/helpers/worldActions.h"
#include "ui/helpers/worldCommands.h"
#include "ui/layouts/InGameLayout.h"

namespace layers {

ui::ButtonWorldAction*
WorldInputController::findWorldActionButton(state::WorldActionType actionType) {
  auto* inGameLayout = owner.getUiElement<ui::InGameLayout>("inGameLayout");
  if (!inGameLayout) {
    return nullptr;
  }
  auto* actionButtons = inGameLayout->getChildById("actionButtons");
  if (!actionButtons) {
    return nullptr;
  }
  for (auto& childPtr : actionButtons->getChildren()) {
    auto* button = dynamic_cast<ui::ButtonWorldAction*>(childPtr.get());
    if (button != nullptr && button->getProps().worldActionType == actionType) {
      return button;
    }
  }
  return nullptr;
}

void WorldInputController::beginWorldActionKeyboardPress(
    state::WorldActionType actionType) {
  auto* stateManager = getStateManager();
  if (!stateManager || worldActionFlash.isBusy()) {
    return;
  }
  // Activate immediately (flash is visual-only) for aim modes and Fight (f);
  // other actions still fire after the flash.
  const bool activateImmediately =
      actionType == state::WorldActionType::EXAMINE ||
      actionType == state::WorldActionType::TALK ||
      actionType == state::WorldActionType::INTERACT ||
      actionType == state::WorldActionType::START_FIGHT ||
      actionType == state::WorldActionType::END_FIGHT;
  worldActionFlash.begin(
      [this, actionType]() -> bool* {
        if (auto* button = findWorldActionButton(actionType)) {
          return &button->isActive;
        }
        return nullptr;
      },
      [this, actionType]() {
        auto* sm = getStateManager();
        if (!sm) {
          return;
        }
        ui::activateWorldAction(*sm, actionType, owner.getWindow());
      },
      owner.getWindow(),
      nullptr,
      !activateImmediately);
}

void WorldInputController::onKeyDown(std::string_view key, int /*keyCode*/) {
  if (owner.getState() != LayerState::ON) {
    return;
  }
  auto stateManager = getStateManager();
  if (!stateManager) {
    return;
  }
  auto& world = stateManager->getState().world;

  if (worldActionFlash.isBusy()) {
    return;
  }

  if (ui::isCancelActionKey(key)) {
    bmin::List<model::WorldActionMode> cancellableActionModes = {
        model::WorldActionMode::EXAMINE,
        model::WorldActionMode::TALK,
        model::WorldActionMode::SPELL,
        model::WorldActionMode::INTERACT,
    };
    if (cancellableActionModes.contains(world.actionMode)) {
      ui::playButtonSound(owner.getWindow());
      ui::cancelCurrentWorldActionMode(*stateManager);
      return;
    }
  }

  if (ui::isWaitKey(key)) {
    ui::enqueueWait(*stateManager);
    return;
  }

  // Block world-action shortcuts / aim confirm while town AI is resolving.
  if (!world.resolvingTownEnemyAi) {
    // `r` always opens magic setup (not remapped through combat Ability → cast).
    if (ui::isOpenMagicSetupKey(key)) {
      ui::playButtonSound(owner.getWindow());
      ui::showMagicSetupLayer(*stateManager, owner.getWindow());
      return;
    }
    if (ui::isOpenSpellCastKey(key)) {
      ui::playButtonSound(owner.getWindow());
      ui::showSpellCastLayer(*stateManager, owner.getWindow());
      return;
    }
    if (auto actionType = ui::getWorldActionFromKeyboardShortcut(
            key, stateManager->getState().turnMode)) {
      beginWorldActionKeyboardPress(*actionType);
      return;
    }
  }

  const bool isAimMode = world.actionMode == model::WorldActionMode::EXAMINE ||
                         world.actionMode == model::WorldActionMode::TALK ||
                         world.actionMode == model::WorldActionMode::SPELL ||
                         world.actionMode == model::WorldActionMode::INTERACT;

  if (isAimMode && ui::isConfirmActionKey(key)) {
    if (!world.resolvingTownEnemyAi && world.actionAimTile) {
      ui::playButtonSound(owner.getWindow());
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
      actionMode != model::WorldActionMode::SPELL &&
      actionMode != model::WorldActionMode::INTERACT) {
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
          actionMode == model::WorldActionMode::SPELL ||
          actionMode == model::WorldActionMode::INTERACT) {
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
  worldActionFlash.update(deltaTime);

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
