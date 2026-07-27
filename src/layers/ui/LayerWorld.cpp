#include "LayerWorld.h"
#include "layers/LayerManager.h"
#include "layers/ui/LayerInventory.h"
#include "model/Combat.h"
#include "model/instances/CharacterPlayer.h"
#include "sdl2w/L10n.h"
#include "sdl2w/Logger.h"
#include "state/LayerManagerInterface.h"
#include "state/WorldActions.h"
#include "state/WorldUpdater.h"
#include "state/actions/combat/DoCombatAction.hpp"
#include "state/actions/combat/EndCombat.hpp"
#include "state/actions/combat/StartCombat.hpp"
#include "state/actions/ui/heldMove/UiUpdateHeldMove.hpp"
#include "state/actions/world/WorldExamineAt.hpp"
#include "state/actions/world/WorldMoveActionAim.hpp"
#include "state/actions/world/WorldMovePlayer.hpp"
#include "state/actions/world/WorldSetActionAim.hpp"
#include "state/actions/world/WorldTalkAt.hpp"
#include "ui/components/FloatingNotificationSection.h"
#include "ui/components/InGameTitleBar.h"
#include "ui/components/MapView.h"
#include "ui/elements/TextLine.h"
#include "ui/elements/buttons/ButtonWorldAction.h"
#include "ui/helpers/keyboardShortcuts.h"
#include "ui/helpers/worldActions.h"
#include "ui/layouts/InGameLayout.h"
#include "ui/observers/ObserverCancelWorldActionMode.hpp"
#include "ui/observers/ObserverWorldAction.hpp"
#include <string_view>

namespace layers {

LayerWorld::LayerWorld(sdl2w::Window* _window) : Layer(_window, LAYER_ID) {
  if (!assertInterfaces()) {
    remove();
    return;
  }

  auto inGameLayout = new ui::InGameLayout(window);
  inGameLayout->setId("inGameLayout");

  auto [windowWidth, windowHeight] = window->getDims();
  const auto scale = 1.f;

  inGameLayout->setPos(0, 0);
  inGameLayout->setScale(scale);

  auto layoutInitProps = inGameLayout->getProps();
  layoutInitProps.width = static_cast<int>(windowWidth / scale);
  layoutInitProps.height = static_cast<int>(windowHeight / scale);
  layoutInitProps.actionButtonScale = 1.5f;
  layoutInitProps.borderType = ui::InGameBorderType::Wide;
  inGameLayout->setProps(layoutInitProps);

  auto titleBar = new ui::InGameTitleBar(window);
  titleBar->setProps(ui::InGameTitleBarProps{
      .title = "World",
      .day = 0,
      .food = 0,
      .ap = 0,
      .showAp = false,
  });
  inGameLayout->setTitleElement(titleBar);

  // Map under action buttons: layer draws uiElements in order, so MapView first.
  auto mapView = new ui::MapView(window);
  mapView->setId("mapView");
  alignMapView();
  addUiElement(mapView);
  addUiElement(inGameLayout);

  auto floatingNotificationSection = new ui::FloatingNotificationSection(window);
  floatingNotificationSection->setId("floatingNotificationSection");
  addUiElement(floatingNotificationSection);

  subscribeAction<state::actions::StartCombat>([this](auto&, auto&) { syncFromState(); });
  subscribeAction<state::actions::EndCombat>([this](auto&, auto&) { syncFromState(); });

  syncFromState();
}

namespace {

bool canPlayerIssueCombatMove(const state::State& state) {
  const auto& world = state.world;
  if (!world.combat.active || !world.combat.isWaitingForAction) {
    return false;
  }
  return model::isPartyMember(state.player, world.combat.activeCharacterId);
}

void enqueueMapMove(state::StateManager& stateManager, int dx, int dy) {
  auto& state = stateManager.getState();
  if (state.world.combat.active) {
    if (!canPlayerIssueCombatMove(state)) {
      return;
    }
    stateManager.enqueueAction(
        stateManager.getActionData(),
        new state::actions::DoCombatAction(model::CombatActionType::MOVE, dx, dy),
        0);
    return;
  }
  stateManager.enqueueAction(stateManager.getActionData(),
                             new state::actions::WorldMovePlayer(dx, dy),
                             0);
}

void enqueueCombatWait(state::StateManager& stateManager) {
  if (!canPlayerIssueCombatMove(stateManager.getState())) {
    return;
  }
  stateManager.enqueueAction(
      stateManager.getActionData(),
      new state::actions::DoCombatAction(model::CombatActionType::WAIT),
      0);
}

} // namespace

void LayerWorld::syncCombatTitleBar() {
  auto inGameLayout = getUiElement<ui::InGameLayout>("inGameLayout");
  if (!inGameLayout) {
    return;
  }
  auto* titleBar = dynamic_cast<ui::InGameTitleBar*>(inGameLayout->getTitleElement());
  if (!titleBar) {
    return;
  }

  auto stateManager = getStateManager();
  if (!stateManager) {
    return;
  }

  const auto& world = stateManager->getState().world;
  auto titleProps = titleBar->getProps();
  const bool showAp = world.combat.active;
  int ap = 0;
  if (showAp) {
    if (const auto* character =
            model::findCharacterOnMap(world.currentMap, world.combat.activeCharacterId)) {
      ap = character->currentAp;
    }
  }
  if (titleProps.showAp != showAp || titleProps.ap != ap) {
    titleProps.showAp = showAp;
    titleProps.ap = ap;
    titleBar->setProps(titleProps);
  }
}

void LayerWorld::confirmWorldActionAim(int tileX, int tileY) {
  auto* stateManager = getStateManager();
  if (!stateManager) {
    return;
  }
  const auto actionMode = stateManager->getState().world.actionMode;
  if (actionMode == model::WorldActionMode::EXAMINE) {
    ui::setHeldMoveActive(*stateManager, false);
    stateManager->enqueueAction(stateManager->getActionData(),
                                new state::actions::WorldExamineAt(tileX, tileY),
                                0);
    return;
  }
  if (actionMode == model::WorldActionMode::TALK) {
    ui::setHeldMoveActive(*stateManager, false);
    stateManager->enqueueAction(
        stateManager->getActionData(), new state::actions::WorldTalkAt(tileX, tileY), 0);
  }
}

void LayerWorld::onKeyDown(std::string_view key, int /*keyCode*/) {
  if (getState() != LayerState::ON) {
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
    };
    if (cancellableActionModes.contains(world.actionMode)) {
      ui::cancelCurrentWorldActionMode(*stateManager);
      return;
    }
  }

  if (ui::isCombatWaitKey(key) && world.combat.active) {
    ui::setHeldMoveActive(*stateManager, false);
    enqueueCombatWait(*stateManager);
    return;
  }

  if (auto actionType =
          ui::getWorldActionFromKeyboardShortcut(key, world.currentMap.turnMode)) {
    ui::activateWorldAction(*stateManager, *actionType, window);
    return;
  }

  const bool isAimMode = world.actionMode == model::WorldActionMode::EXAMINE ||
                         world.actionMode == model::WorldActionMode::TALK;

  if (isAimMode && ui::isConfirmActionKey(key)) {
    if (world.actionAimTile) {
      confirmWorldActionAim(world.actionAimTile->x, world.actionAimTile->y);
    }
    return;
  }

  auto moveDelta = ui::getMoveDeltaForKey(key);
  if (!moveDelta) {
    return;
  }

  if (world.combat.active && !canPlayerIssueCombatMove(stateManager->getState())) {
    return;
  }

  if (isAimMode) {
    ui::setHeldMoveActive(*stateManager, false);
    stateManager->enqueueAction(
        stateManager->getActionData(),
        new state::actions::WorldMoveActionAim(moveDelta->dx, moveDelta->dy),
        0);
    return;
  }

  const auto& heldMove = stateManager->getState().uiState.heldMove;
  // Ignore OS/SDL key-repeat events for the same held key; we time repeats ourselves.
  if (heldMove.isActive && heldMove.key == key) {
    return;
  }

  state::HeldMove nextHeldMove{
      .isActive = true,
      .key = bmin::fromStringView(key),
      .dx = moveDelta->dx,
      .dy = moveDelta->dy,
  };
  model::timerStructRestart(nextHeldMove.initialDelay);
  model::timerStructRestart(nextHeldMove.moveDelay);
  stateManager->pllAction(stateManager->getActionData(),
                          new state::actions::UiUpdateHeldMove(nextHeldMove),
                          0);
  enqueueMapMove(*stateManager, moveDelta->dx, moveDelta->dy);
}

void LayerWorld::onKeyUp(std::string_view key, int /*keyCode*/) {
  auto stateManager = getStateManager();
  if (!stateManager) {
    return;
  }
  const auto& heldMove = stateManager->getState().uiState.heldMove;
  if (heldMove.isActive && heldMove.key == key) {
    ui::setHeldMoveActive(*stateManager, false);
  }
}

void LayerWorld::updateAimFromMouse(int x, int y) {
  if (getState() != LayerState::ON) {
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
      actionMode != model::WorldActionMode::TALK) {
    return;
  }
  auto* mapView = getUiElement<ui::MapView>("mapView");
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
  stateManager->pllAction(stateManager->getActionData(),
                          new state::actions::WorldSetActionAim(tile->x, tile->y),
                          0);
}

void LayerWorld::onMouseHover(int x, int y) {
  updateAimFromMouse(x, y);
  Layer::onMouseHover(x, y);
}

void LayerWorld::onMouseDown(int x, int y, int button) {
  // SDL_BUTTON_LEFT == 1
  if (getState() == LayerState::ON && button == 1) {
    auto* stateManager = getStateManager();
    if (stateManager) {
      const auto actionMode = stateManager->getState().world.actionMode;
      if (actionMode == model::WorldActionMode::EXAMINE ||
          actionMode == model::WorldActionMode::TALK) {
        if (auto* mapView = getUiElement<ui::MapView>("mapView")) {
          if (auto tile = mapView->screenToTile(x, y)) {
            stateManager->enqueueAction(
                stateManager->getActionData(),
                new state::actions::WorldSetActionAim(tile->x, tile->y),
                0);
            confirmWorldActionAim(tile->x, tile->y);
            return;
          }
        }
      }
    }
  }
  Layer::onMouseDown(x, y, button);
}

void LayerWorld::alignMapView() {
  auto inGameLayout = getUiElement<ui::InGameLayout>("inGameLayout");
  auto mapView = getUiElement<ui::MapView>("mapView");
  auto world = &getStateManager()->getState().world;
  if (!inGameLayout || !mapView || !world) {
    return;
  }
  auto [worldX, worldY] = inGameLayout->getWorldLocation();
  auto [worldW, worldH] = inGameLayout->getWorldDims();
  // getWorldLocation/Dims are already in screen pixels (scaled).
  mapView->setPos(worldX, worldY);
  mapView->setScale(mapScale);
  mapView->setProps(ui::MapViewProps{
      .width = static_cast<int>(worldW / mapScale),
      .height = static_cast<int>(worldH / mapScale),
  });
  // Content dims are in map pixels; screen size is content * mapScale.
  world->camera.viewW = static_cast<int>(worldW / mapScale);
  world->camera.viewH = static_cast<int>(worldH / mapScale);
}

void LayerWorld::setMapScale(float scale) {
  mapScale = scale;
  alignMapView();
}

void LayerWorld::fillWorldActionTypes(model::TurnMode turnMode,
                                      bmin::DynArray<state::WorldActionType>& dest) {
  auto copyActionTypes = [](bmin::DynArray<state::WorldActionType>& dest,
                            const auto& source) {
    dest.clear();
    for (const auto& type : source) {
      dest.pushBack(type);
    }
  };

  const state::WorldActionUiState worldActionUiState;
  switch (turnMode) {
  case model::TurnMode::TURN_OUTDOOR:
    copyActionTypes(dest, worldActionUiState.outdoorModeActionTypes);
    break;
  case model::TurnMode::TURN_COMBAT:
    copyActionTypes(dest, worldActionUiState.townModeFightActionTypes);
    break;
  case model::TurnMode::TURN_TOWN:
  default:
    copyActionTypes(dest, worldActionUiState.townModeActionTypes);
    break;
  }
}

void LayerWorld::attachWorldActionObservers(ui::InGameLayout* inGameLayout) {
  if (!inGameLayout) {
    return;
  }

  auto* stateManager = getStateManager();
  if (!stateManager) {
    return;
  }

  auto* actionButtons = inGameLayout->getChildById("actionButtons");
  if (!actionButtons) {
    return;
  }

  for (auto& childPtr : actionButtons->getChildren()) {
    auto* button = dynamic_cast<ui::ButtonWorldAction*>(childPtr.get());
    if (!button) {
      continue;
    }
    button->addEventObserver(new ui::ObserverWorldAction(
        stateManager, button->getProps().worldActionType, window));
  }
}

void LayerWorld::syncWorldActionModeHighlight() {
  auto inGameLayout = getUiElement<ui::InGameLayout>("inGameLayout");
  if (!inGameLayout || !assertInterfaces()) {
    return;
  }

  const auto actionMode = getStateManager()->getState().world.actionMode;
  auto* layerManager = state::LayerManagerInterface::getLayerManager();
  const bool inventoryOpen =
      layerManager != nullptr &&
      layerManager->getLayerById(LayerInventory::LAYER_ID) != nullptr;
  auto* actionButtons = inGameLayout->getChildById("actionButtons");
  if (!actionButtons) {
    return;
  }

  for (auto& childPtr : actionButtons->getChildren()) {
    auto* button = dynamic_cast<ui::ButtonWorldAction*>(childPtr.get());
    if (!button) {
      continue;
    }
    const auto actionType = button->getProps().worldActionType;
    const bool modeSelected =
        (actionType == state::WorldActionType::EXAMINE &&
         actionMode == model::WorldActionMode::EXAMINE) ||
        (actionType == state::WorldActionType::TALK &&
         actionMode == model::WorldActionMode::TALK) ||
        (actionType == state::WorldActionType::INVENTORY && inventoryOpen);
    if (button->isModeSelected != modeSelected) {
      button->isModeSelected = modeSelected;
    }
  }
}

void LayerWorld::syncActionModeCancelButton() {
  auto inGameLayout = getUiElement<ui::InGameLayout>("inGameLayout");
  if (!inGameLayout || !assertInterfaces()) {
    return;
  }

  const auto actionMode = getStateManager()->getState().world.actionMode;
  bmin::String modeLabel;
  if (actionMode == model::WorldActionMode::EXAMINE) {
    modeLabel = TRANSLATE("Examine");
  } else if (actionMode == model::WorldActionMode::TALK) {
    modeLabel = TRANSLATE("Talk");
  }

  const bool shouldShow = !modeLabel.empty();
  auto* existingCancel = inGameLayout->getChildById("actionModeCancel");
  auto* existingLabel =
      dynamic_cast<ui::TextLine*>(inGameLayout->getChildById("actionModeLabel"));
  if (shouldShow && existingCancel && existingLabel) {
    const auto& labelProps = existingLabel->getProps();
    if (!labelProps.textBlocks.empty() && labelProps.textBlocks[0].text == modeLabel) {
      return;
    }
  } else if (!shouldShow && !existingCancel && !existingLabel) {
    return;
  }

  inGameLayout->setActionModeCancelVisible(shouldShow, modeLabel);
  if (shouldShow) {
    if (auto* cancelButton = inGameLayout->getChildById("actionModeCancel")) {
      cancelButton->addEventObserver(
          new ui::ObserverCancelWorldActionMode(getStateManager()));
    }
  }
}

void LayerWorld::syncFromState() {
  if (!assertInterfaces()) {
    remove();
    return;
  }

  auto inGameLayout = getUiElement<ui::InGameLayout>("inGameLayout");
  if (!inGameLayout) {
    LOG(ERROR) << "LayerWorld::syncFromState: inGameLayout is nullptr" << LOG_ENDL;
    return;
  }

  auto stateManager = getStateManager();
  auto& state = stateManager->getState();
  auto& player = state.player;
  auto& world = state.world;

  auto layoutProps = inGameLayout->getProps();
  fillWorldActionTypes(world.currentMap.turnMode, layoutProps.worldActionTypes);
  layoutProps.partyMembers.clear();
  for (int i = 0; i < static_cast<int>(player.party.size()); i++) {
    const auto& member = player.party[i];
    ui::ChCompactInfoProps entry;
    entry.characterSpriteName = model::characterPlayerGetSprite(member);
    entry.hp = member.currentHp;
    entry.mana = member.currentMp;
    entry.isSelected = (i == player.currentPartyMemberIndex);
    layoutProps.partyMembers.pushBack(entry);
  }
  inGameLayout->setProps(layoutProps);
  attachWorldActionObservers(inGameLayout);
  syncWorldActionModeHighlight();
  syncActionModeCancelButton();

  if (auto* titleBar =
          dynamic_cast<ui::InGameTitleBar*>(inGameLayout->getTitleElement())) {
    auto titleProps = titleBar->getProps();
    titleProps.title = world.name.empty() ? bmin::String("World") : world.name;
    // day/ap placeholders until those fields live on State
    titleProps.day = 0;
    titleProps.food = player.food;
    titleProps.ap = 0;
    if (world.combat.active) {
      if (const auto* character = model::findCharacterOnMap(
              world.currentMap, world.combat.activeCharacterId)) {
        titleProps.ap = character->currentAp;
      }
    }
    titleProps.showAp = world.combat.active;
    titleBar->setProps(titleProps);
  }

  alignMapView();
}

void LayerWorld::updateHeldMoveRepeat(int deltaTime) {
  auto stateManager = getStateManager();
  if (!stateManager) {
    return;
  }

  auto& heldMove = stateManager->getState().uiState.heldMove;
  if (!heldMove.isActive) {
    return;
  }

  if (stateManager->getState().world.actionMode != model::WorldActionMode::NONE) {
    ui::setHeldMoveActive(*stateManager, false);
    return;
  }

  if (stateManager->getState().world.combat.active &&
      !canPlayerIssueCombatMove(stateManager->getState())) {
    ui::setHeldMoveActive(*stateManager, false);
    return;
  }

  model::timerStructUpdate(heldMove.initialDelay, deltaTime);
  model::timerStructUpdate(heldMove.moveDelay, deltaTime);

  if (model::timerStructIsComplete(heldMove.initialDelay) &&
      model::timerStructIsComplete(heldMove.moveDelay)) {
    model::timerStructRestart(heldMove.moveDelay);
    enqueueMapMove(*stateManager, heldMove.dx, heldMove.dy);
  }
}

// void LayerWorld::processPendingTriggers() {
//   auto stateManager = getStateManager();
//   if (!stateManager) {
//     return;
//   }

//   auto& state = stateManager->getState();
//   auto& world = state.world;
//   bool mapChanged = false;

//   if (world.pendingSpecialEventId) {
//     ui::setHeldMoveActive(*stateManager, false);
//     auto eventId = *world.pendingSpecialEventId;
//     world.pendingSpecialEventId.reset();
//     state::actions::UiShowLayerSpecialEvent specialEvent =
//         state::actions::UiShowLayerSpecialEvent(window, eventId);
//     specialEvent.execute(&state);
//   }

//   if (world.pendingTravel) {
//     ui::setHeldMoveActive(*stateManager, false);
//     auto travel = *world.pendingTravel;
//     world.pendingTravel.reset();
//     state::actions::WorldTravel travelAction(travel);
//     travelAction.execute(&state);
//     mapChanged = true;
//   }

//   if (mapChanged) {
//     syncFromState();
//   }
// }

void LayerWorld::update(int deltaTime) {
  Layer::update(deltaTime);
  // Hover is polled here: LayerManager has no mouse-move dispatch, and tests/game
  // only wire down/up/wheel. mouseX/Y are updated by SDL every frame.
  auto& events = window->getEvents();
  updateAimFromMouse(events.mouseX, events.mouseY);
  updateHeldMoveRepeat(deltaTime);

  auto stateManager = getStateManager();
  if (stateManager) {
    state::worldProcessPendingTriggers(window, *stateManager);
    if (stateManager->getState().world.mapChangedThisTick) {
      syncFromState();
    }
  }

  syncWorldActionModeHighlight();
  syncActionModeCancelButton();
  syncCombatTitleBar();
}

void LayerWorld::render(int deltaTime) {
  // World is SUSPENDED while inventory is open (update does not run); still refresh
  // the inventory action button pressed state before drawing.
  syncWorldActionModeHighlight();
  Layer::render(deltaTime);
}

} // namespace layers
