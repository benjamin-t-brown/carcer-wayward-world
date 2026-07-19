#include "LayerWorld.h"
#include "model/instances/CharacterPlayer.h"
#include "sdl2w/L10n.h"
#include "sdl2w/Logger.h"
#include "state/WorldActions.h"
#include "state/actions/ui/UiShowLayerSpecialEvent.hpp"
#include "state/actions/ui/heldMove/UiUpdateHeldMove.hpp"
#include "state/actions/world/WorldExamineDirection.hpp"
#include "state/actions/world/WorldMovePlayer.hpp"
#include "state/actions/world/WorldTalkDirection.hpp"
#include "state/actions/world/WorldTravel.hpp"
#include "ui/components/FloatingNotificationSection.h"
#include "ui/components/InGameTitleBar.h"
#include "ui/components/MapView.h"
#include "ui/elements/TextLine.h"
#include "ui/elements/buttons/ButtonWorldAction.h"
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

  syncFromState();
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

  if (key == "Escape") {
    bmin::List<model::WorldActionMode> cancellableActionModes = {
        model::WorldActionMode::EXAMINE,
        model::WorldActionMode::TALK,
    };
    if (cancellableActionModes.contains(world.actionMode)) {
      ui::cancelCurrentWorldActionMode(*stateManager);
      return;
    }
  }

  if (auto actionType = ui::getWorldActionFromKeyboardShortcut(key)) {
    ui::activateWorldAction(*stateManager, *actionType);
    return;
  }

  auto moveDelta = getMoveDeltaForKey(key);
  if (!moveDelta) {
    return;
  }

  if (world.actionMode == model::WorldActionMode::EXAMINE) {
    ui::setHeldMoveActive(*stateManager, false);
    stateManager->enqueueAction(
        stateManager->getActionData(),
        new state::actions::WorldExamineDirection(moveDelta->dx, moveDelta->dy),
        0);
    return;
  }

  if (world.actionMode == model::WorldActionMode::TALK) {
    ui::setHeldMoveActive(*stateManager, false);
    stateManager->enqueueAction(
        stateManager->getActionData(),
        new state::actions::WorldTalkDirection(moveDelta->dx, moveDelta->dy),
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
  stateManager->pllAction(
      stateManager->getActionData(),
      new state::actions::WorldMovePlayer(moveDelta->dx, moveDelta->dy),
      0);
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

std::optional<LayerWorld::MoveDelta>
LayerWorld::getMoveDeltaForKey(std::string_view key) {
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
  world->viewW = static_cast<int>(worldW / mapScale);
  world->viewH = static_cast<int>(worldH / mapScale);
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
    button->addEventObserver(
        new ui::ObserverWorldAction(stateManager, button->getProps().worldActionType));
  }
}

void LayerWorld::syncWorldActionModeHighlight() {
  auto inGameLayout = getUiElement<ui::InGameLayout>("inGameLayout");
  if (!inGameLayout || !assertInterfaces()) {
    return;
  }

  const auto actionMode = getStateManager()->getState().world.actionMode;
  auto* actionButtons = inGameLayout->getChildById("actionButtons");
  if (!actionButtons) {
    return;
  }

  for (auto& childPtr : actionButtons->getChildren()) {
    auto* button = dynamic_cast<ui::ButtonWorldAction*>(childPtr.get());
    if (!button) {
      continue;
    }
    const bool modeSelected =
        (button->getProps().worldActionType == state::WorldActionType::EXAMINE &&
         actionMode == model::WorldActionMode::EXAMINE) ||
        (button->getProps().worldActionType == state::WorldActionType::TALK &&
         actionMode == model::WorldActionMode::TALK);
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
    titleProps.showAp = (world.currentMap.turnMode == model::TurnMode::TURN_COMBAT);
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

  model::timerStructUpdate(heldMove.initialDelay, deltaTime);
  model::timerStructUpdate(heldMove.moveDelay, deltaTime);

  if (model::timerStructIsComplete(heldMove.initialDelay) &&
      model::timerStructIsComplete(heldMove.moveDelay)) {
    model::timerStructRestart(heldMove.moveDelay);
    stateManager->enqueueAction(
        stateManager->getActionData(),
        new state::actions::WorldMovePlayer(heldMove.dx, heldMove.dy),
        0);
  }
}

void LayerWorld::processPendingTriggers() {
  auto stateManager = getStateManager();
  if (!stateManager) {
    return;
  }

  auto& state = stateManager->getState();
  auto& world = state.world;
  bool mapChanged = false;

  if (world.pendingSpecialEventId) {
    ui::setHeldMoveActive(*stateManager, false);
    auto eventId = *world.pendingSpecialEventId;
    world.pendingSpecialEventId.reset();
    state::actions::UiShowLayerSpecialEvent specialEvent =
        state::actions::UiShowLayerSpecialEvent(window, eventId);
    specialEvent.execute(&state);
  }

  if (world.pendingTravel) {
    ui::setHeldMoveActive(*stateManager, false);
    auto travel = *world.pendingTravel;
    world.pendingTravel.reset();
    state::actions::WorldTravel travelAction(travel);
    travelAction.execute(&state);
    mapChanged = true;
  }

  if (mapChanged) {
    syncFromState();
  }
}

void LayerWorld::update(int deltaTime) {
  Layer::update(deltaTime);
  updateHeldMoveRepeat(deltaTime);
  processPendingTriggers();
  syncWorldActionModeHighlight();
  syncActionModeCancelButton();
}

} // namespace layers
