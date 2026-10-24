#include "WorldInputController.h"
#include "bmin/StringInterop.h"
#include "game/map/MapPickup.h"
#include "game/map/TileTriggers.h"
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
#include <cstdlib>

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

bool WorldInputController::partyHasNearbyPickupItems() {
  auto* stateManager = getStateManager();
  auto* database = getDatabase();
  if (stateManager == nullptr || database == nullptr) {
    return false;
  }
  auto& state = stateManager->getState();
  const auto* avatar =
      game::findPartyAvatarOnActiveMap(state.world.activeMap, state.player);
  if (avatar == nullptr) {
    return false;
  }
  return !game::collectItemsWithinPickupRange(state.world.activeMap,
                                             state.mapInstances,
                                             *avatar,
                                             game::PICKUP_PATH_RANGE,
                                             *database)
              .empty();
}

void WorldInputController::beginWorldActionKeyboardPress(
    state::WorldActionType actionType) {
  auto* stateManager = getStateManager();
  if (!stateManager || worldActionFlash.isBusy()) {
    return;
  }
  // Flash is visual-only for these. Get waits for the flash only when there is
  // nothing nearby (button press + "no items" notice, no window).
  const bool activateImmediately =
      actionType == state::WorldActionType::EXAMINE ||
      actionType == state::WorldActionType::TALK ||
      actionType == state::WorldActionType::INTERACT ||
      actionType == state::WorldActionType::START_FIGHT ||
      actionType == state::WorldActionType::END_FIGHT ||
      actionType == state::WorldActionType::INVENTORY ||
      actionType == state::WorldActionType::ABILITY ||
      actionType == state::WorldActionType::SHOOT ||
      (actionType == state::WorldActionType::GET && partyHasNearbyPickupItems());
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

void WorldInputController::confirmInteractAt(int tileX, int tileY) {
  auto* stateManager = getStateManager();
  if (!stateManager) {
    return;
  }
  ui::setHeldMoveActive(*stateManager, false);
  ui::confirmWorldActionAim(*stateManager, owner.getWindow(), getDatabase(), tileX,
                            tileY);
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
        model::WorldActionMode::SHOOT,
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

  // Roam-aim modes move a highlight box; INTERACT is adjacent-pick (no aim tile).
  const bool isRoamAimMode =
      world.actionMode == model::WorldActionMode::EXAMINE ||
      world.actionMode == model::WorldActionMode::TALK ||
      world.actionMode == model::WorldActionMode::SPELL ||
      world.actionMode == model::WorldActionMode::SHOOT;
  const bool isInteractPending =
      world.actionMode == model::WorldActionMode::INTERACT;

  if (isInteractPending && ui::isConfirmActionKey(key)) {
    if (!world.resolvingTownEnemyAi) {
      const auto* avatar =
          game::findPartyAvatarOnActiveMap(world.activeMap, stateManager->getState().player);
      if (avatar) {
        confirmInteractAt(avatar->x, avatar->y);
      }
    }
    return;
  }

  if (isRoamAimMode && ui::isConfirmActionKey(key)) {
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

  if (isInteractPending) {
    if (world.resolvingTownEnemyAi) {
      return;
    }
    const auto* avatar =
        game::findPartyAvatarOnActiveMap(world.activeMap, stateManager->getState().player);
    if (!avatar) {
      return;
    }
    confirmInteractAt(avatar->x + moveDelta->dx, avatar->y + moveDelta->dy);
    return;
  }

  if (isRoamAimMode) {
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
  // INTERACT is adjacent-pick; do not roam the aim highlight with the mouse.
  if (actionMode != model::WorldActionMode::EXAMINE &&
      actionMode != model::WorldActionMode::TALK &&
      actionMode != model::WorldActionMode::SPELL &&
      actionMode != model::WorldActionMode::SHOOT) {
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
      const auto& state = stateManager->getState();
      const auto actionMode = state.world.actionMode;
      if (actionMode == model::WorldActionMode::INTERACT) {
        if (auto* mapView = owner.getUiElement<ui::MapView>("mapView")) {
          if (auto tile = mapView->screenToTile(x, y)) {
            // Adjacent or not: WorldInteractAt cancels + "Too far." when out of range.
            confirmInteractAt(tile->x, tile->y);
            return true;
          }
        }
        return false;
      }
      if (actionMode == model::WorldActionMode::EXAMINE ||
          actionMode == model::WorldActionMode::TALK ||
          actionMode == model::WorldActionMode::SPELL ||
          actionMode == model::WorldActionMode::SHOOT) {
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
