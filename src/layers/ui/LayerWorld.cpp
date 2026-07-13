#include "LayerWorld.h"
#include "sdl2w/Logger.h"
#include "model/instances/CharacterPlayer.h"
#include "state/WorldActions.h"
#include "state/actions/ui/UiShowLayerSpecialEvent.hpp"
#include "state/actions/world/WorldExamineDirection.hpp"
#include "state/actions/world/WorldMovePlayer.hpp"
#include "state/actions/world/WorldSetActionMode.hpp"
#include "state/actions/world/WorldTravel.hpp"
#include "ui/components/FloatingNotificationSection.h"
#include "ui/components/InGameTitleBar.h"
#include "ui/components/MapView.h"
#include "ui/elements/buttons/ButtonWorldAction.h"
#include "ui/layouts/InGameLayout.h"
#include "ui/observers/ObserverWorldAction.hpp"

namespace layers {
namespace {

void copyActionTypes(bmin::DynArray<state::WorldActionType>& dest, const auto& source) {
  dest.clear();
  for (const auto& type : source) {
    dest.pushBack(type);
  }
}

void fillWorldActionTypes(model::TurnMode turnMode,
                          bmin::DynArray<state::WorldActionType>& dest) {
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

void layoutMapView(ui::InGameLayout* inGameLayout,
                   ui::MapView* mapView,
                   model::World* world) {
  if (!inGameLayout || !mapView) {
    return;
  }
  auto [worldX, worldY] = inGameLayout->getWorldLocation();
  auto [worldW, worldH] = inGameLayout->getWorldDims();
  // getWorldLocation/Dims are already in screen pixels (scaled).
  mapView->setPos(worldX, worldY);
  mapView->setScale(1.f);
  mapView->setProps(ui::MapViewProps{
      .width = worldW,
      .height = worldH,
  });
  if (world) {
    // With MapView scale 1, content dims match map-pixel viewport size.
    world->viewW = worldW;
    world->viewH = worldH;
  }
}

} // namespace

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

  // Map under chrome: layer draws uiElements in order, so MapView first.
  auto mapView = new ui::MapView(window);
  mapView->setId("mapView");
  layoutMapView(inGameLayout, mapView, &getStateManager()->getState().world);
  addUiElement(mapView);
  addUiElement(inGameLayout);

  auto floatingNotificationSection = new ui::FloatingNotificationSection(window);
  floatingNotificationSection->setId("floatingNotificationSection");
  addUiElement(floatingNotificationSection);

  syncFromState();
}

void LayerWorld::onKeyDown(std::string_view key, int keyCode) {
  if (getState() != LayerState::ON) {
    return;
  }

  auto stateManager = getStateManager();
  if (!stateManager) {
    return;
  }

  auto& world = stateManager->getState().world;
  if (key == "Escape" && world.actionMode == model::WorldActionMode::EXAMINE) {
    stateManager->enqueueAction(
        stateManager->getActionData(),
        new state::actions::WorldSetActionMode(model::WorldActionMode::NONE),
        0);
    return;
  }

  if (auto actionType = worldActionShortcutForKey(key)) {
    activateWorldAction(*actionType);
    return;
  }

  auto dx = int{0};
  auto dy = int{0};
  if (key == "Up") {
    dy = -1;
  } else if (key == "Down") {
    dy = 1;
  } else if (key == "Left") {
    dx = -1;
  } else if (key == "Right") {
    dx = 1;
  } else {
    return;
  }

  if (world.actionMode == model::WorldActionMode::EXAMINE) {
    stateManager->enqueueAction(stateManager->getActionData(),
                                new state::actions::WorldExamineDirection(dx, dy),
                                0);
    return;
  }

  stateManager->enqueueAction(stateManager->getActionData(),
                              new state::actions::WorldMovePlayer(dx, dy),
                              0);
}

std::optional<state::WorldActionType>
LayerWorld::worldActionShortcutForKey(std::string_view key) {
  if (key == "l" || key == "L") {
    return state::WorldActionType::EXAMINE;
  }
  return std::nullopt;
}

void LayerWorld::activateWorldAction(state::WorldActionType worldActionType) {
  auto stateManager = getStateManager();
  if (!stateManager) {
    return;
  }

  switch (worldActionType) {
  case state::WorldActionType::EXAMINE:
    stateManager->enqueueAction(
        stateManager->getActionData(),
        new state::actions::WorldSetActionMode(model::WorldActionMode::EXAMINE),
        0);
    break;
  default:
    break;
  }
}

void LayerWorld::update(int deltaTime) {
  Layer::update(deltaTime);
  processPendingTriggers();
  syncWorldActionModeHighlight();
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
    const bool modeSelected = button->getProps().worldActionType == state::WorldActionType::EXAMINE &&
                              actionMode == model::WorldActionMode::EXAMINE;
    if (button->isModeSelected != modeSelected) {
      button->isModeSelected = modeSelected;
    }
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
    auto eventId = *world.pendingSpecialEventId;
    world.pendingSpecialEventId.reset();
    state::actions::UiShowLayerSpecialEvent showEvent(window, eventId);
    showEvent.execute(&state);
  }

  if (world.pendingTravel) {
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

void LayerWorld::attachWorldActionObservers(ui::InGameLayout* inGameLayout) {
  if (!inGameLayout) {
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
        new ui::ObserverWorldAction(this, button->getProps().worldActionType));
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

  if (auto* titleBar = dynamic_cast<ui::InGameTitleBar*>(inGameLayout->getTitleElement())) {
    auto titleProps = titleBar->getProps();
    titleProps.title = world.name.empty() ? bmin::String("World") : world.name;
    // day/ap placeholders until those fields live on State
    titleProps.day = 0;
    titleProps.food = player.food;
    titleProps.ap = 0;
    titleProps.showAp = (world.currentMap.turnMode == model::TurnMode::TURN_COMBAT);
    titleBar->setProps(titleProps);
  }

  layoutMapView(inGameLayout, getUiElement<ui::MapView>("mapView"), &world);
}

} // namespace layers
