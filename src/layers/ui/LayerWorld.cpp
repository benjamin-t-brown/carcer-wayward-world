#include "LayerWorld.h"
#include "sdl2w/L10n.h"
#include "sdl2w/Logger.h"
#include "model/instances/CharacterPlayer.h"
#include "state/WorldActions.h"
#include "state/actions/ui/UiShowLayerSpecialEvent.hpp"
#include "state/actions/world/WorldExamineDirection.hpp"
#include "state/actions/world/WorldMovePlayer.hpp"
#include "state/actions/world/WorldSetActionMode.hpp"
#include "state/actions/world/WorldTalkDirection.hpp"
#include "state/actions/world/WorldTravel.hpp"
#include "ui/components/FloatingNotificationSection.h"
#include "ui/components/InGameTitleBar.h"
#include "ui/components/MapView.h"
#include "ui/elements/TextLine.h"
#include "ui/elements/buttons/ButtonWorldAction.h"
#include "ui/layouts/InGameLayout.h"
#include "ui/observers/ObserverCancelWorldActionMode.hpp"
#include "ui/observers/ObserverWorldAction.hpp"
#include <string_view>

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

struct MoveDelta {
  int dx = 0;
  int dy = 0;
};

std::optional<MoveDelta> moveDeltaForKey(std::string_view key) {
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

constexpr int kMoveRepeatInitialDelayMs = 300;
constexpr int kMoveRepeatIntervalMs = 50;

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

void LayerWorld::clearHeldMove() {
  heldMove.reset();
}

void LayerWorld::enqueuePlayerMove(int dx, int dy) {
  auto stateManager = getStateManager();
  if (!stateManager) {
    return;
  }
  stateManager->enqueueAction(
      stateManager->getActionData(),
      new state::actions::WorldMovePlayer(dx, dy),
      0);
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
  if (key == "Escape" && (world.actionMode == model::WorldActionMode::EXAMINE ||
                          world.actionMode == model::WorldActionMode::TALK)) {
    cancelWorldActionMode();
    return;
  }

  if (auto actionType = worldActionShortcutForKey(key)) {
    clearHeldMove();
    activateWorldAction(*actionType);
    return;
  }

  auto moveDelta = moveDeltaForKey(key);
  if (!moveDelta) {
    return;
  }

  if (world.actionMode == model::WorldActionMode::EXAMINE) {
    clearHeldMove();
    stateManager->enqueueAction(
        stateManager->getActionData(),
        new state::actions::WorldExamineDirection(moveDelta->dx, moveDelta->dy),
        0);
    return;
  }

  if (world.actionMode == model::WorldActionMode::TALK) {
    clearHeldMove();
    stateManager->enqueueAction(
        stateManager->getActionData(),
        new state::actions::WorldTalkDirection(moveDelta->dx, moveDelta->dy),
        0);
    return;
  }

  // Ignore OS/SDL key-repeat events for the same held key; we time repeats ourselves.
  if (heldMove &&
      std::string_view(heldMove->key.cStr(), heldMove->key.size()) == key) {
    return;
  }

  heldMove = HeldMove{
      .key = bmin::String(key.data(), key.size()),
      .dx = moveDelta->dx,
      .dy = moveDelta->dy,
      .heldMs = 0,
      .repeating = false,
  };
  enqueuePlayerMove(moveDelta->dx, moveDelta->dy);
}

void LayerWorld::onKeyUp(std::string_view key, int /*keyCode*/) {
  if (heldMove &&
      std::string_view(heldMove->key.cStr(), heldMove->key.size()) == key) {
    clearHeldMove();
  }
}

std::optional<state::WorldActionType>
LayerWorld::worldActionShortcutForKey(std::string_view key) {
  if (key == "l" || key == "L") {
    return state::WorldActionType::EXAMINE;
  }
  if (key == "t" || key == "T") {
    return state::WorldActionType::TALK;
  }
  return std::nullopt;
}

void LayerWorld::cancelWorldActionMode() {
  auto stateManager = getStateManager();
  if (!stateManager) {
    return;
  }
  if (stateManager->getState().world.actionMode == model::WorldActionMode::NONE) {
    return;
  }
  clearHeldMove();
  stateManager->enqueueAction(
      stateManager->getActionData(),
      new state::actions::WorldSetActionMode(model::WorldActionMode::NONE),
      0);
}

void LayerWorld::activateWorldAction(state::WorldActionType worldActionType) {
  auto stateManager = getStateManager();
  if (!stateManager) {
    return;
  }

  const auto currentMode = stateManager->getState().world.actionMode;

  switch (worldActionType) {
  case state::WorldActionType::EXAMINE:
    clearHeldMove();
    if (currentMode == model::WorldActionMode::EXAMINE) {
      cancelWorldActionMode();
      break;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(),
        new state::actions::WorldSetActionMode(model::WorldActionMode::EXAMINE),
        0);
    break;
  case state::WorldActionType::TALK:
    clearHeldMove();
    if (currentMode == model::WorldActionMode::TALK) {
      cancelWorldActionMode();
      break;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(),
        new state::actions::WorldSetActionMode(model::WorldActionMode::TALK),
        0);
    break;
  default:
    break;
  }
}

void LayerWorld::updateHeldMoveRepeat(int deltaTime) {
  if (!heldMove || deltaTime <= 0) {
    return;
  }

  auto stateManager = getStateManager();
  if (!stateManager) {
    clearHeldMove();
    return;
  }

  if (stateManager->getState().world.actionMode != model::WorldActionMode::NONE) {
    clearHeldMove();
    return;
  }

  // Stop if the key is no longer down (covers missing KEY_UP wiring / focus loss).
  if (window &&
      !window->getEvents().isKeyPressed(
          std::string_view(heldMove->key.cStr(), heldMove->key.size()))) {
    clearHeldMove();
    return;
  }

  heldMove->heldMs += deltaTime;

  if (!heldMove->repeating) {
    if (heldMove->heldMs < kMoveRepeatInitialDelayMs) {
      return;
    }
    heldMove->repeating = true;
    heldMove->heldMs -= kMoveRepeatInitialDelayMs;
    enqueuePlayerMove(heldMove->dx, heldMove->dy);
  }

  // At most one step per frame so a hitch doesn't dump many moves.
  if (heldMove->repeating && heldMove->heldMs >= kMoveRepeatIntervalMs) {
    heldMove->heldMs -= kMoveRepeatIntervalMs;
    enqueuePlayerMove(heldMove->dx, heldMove->dy);
  }
}

void LayerWorld::update(int deltaTime) {
  Layer::update(deltaTime);
  updateHeldMoveRepeat(deltaTime);
  processPendingTriggers();
  syncWorldActionModeHighlight();
  syncActionModeCancelButton();
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
  auto* existingLabel = dynamic_cast<ui::TextLine*>(inGameLayout->getChildById("actionModeLabel"));
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
      cancelButton->addEventObserver(new ui::ObserverCancelWorldActionMode(this));
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
  syncActionModeCancelButton();

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
