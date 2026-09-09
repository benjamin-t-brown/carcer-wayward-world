#include "LayerWorld.h"
#include "actions/world/WorldUpdater.h"
#include "ui/components/FloatingNotificationSection.h"
#include "ui/components/InGameTitleBar.h"
#include "ui/components/MapView.h"
#include "ui/layouts/InGameLayout.h"
#include <string_view>

namespace layers {

LayerWorld::LayerWorld(sdl2w::Window* _window) : UiLayer(_window, LAYER_ID) {
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
  inGameLayout->setTitleElement(bmin::UniquePtr<ui::UiElement>(titleBar));

  // Map under action buttons: layer draws uiElements in order, so MapView first.
  auto mapView = new ui::MapView(window);
  mapView->setId("mapView");
  alignMapView();
  addUiElement(bmin::UniquePtr<ui::UiElement>(mapView));
  addUiElement(bmin::UniquePtr<ui::UiElement>(inGameLayout));

  auto floatingNotificationSection = new ui::FloatingNotificationSection(window);
  floatingNotificationSection->setId("floatingNotificationSection");
  addUiElement(bmin::UniquePtr<ui::UiElement>(floatingNotificationSection));

  subscribeAction<state::ActionEvent::StartCombat>([this](auto&, auto&) { syncFromState(); });
  subscribeAction<state::ActionEvent::EndCombat>([this](auto&, auto&) { syncFromState(); });
  subscribeAction<state::ActionEvent::SetActiveCombatCharacter>(
      [this](auto&, auto&) { syncFromState(); });
  subscribeAction<state::ActionEvent::UiSetSelectedPartyMemberId>(
      [this](auto&, auto&) { syncFromState(); });
  subscribeAction<state::ActionEvent::ModifyHP>([this](auto&, auto&) { syncFromState(); });
  subscribeAction<state::ActionEvent::ModifyPartyMemberHp>(
      [this](auto&, auto&) { syncFromState(); });

  syncFromState();
}

void LayerWorld::onKeyDown(std::string_view key, int keyCode) {
  inputController.onKeyDown(key, keyCode);
}

void LayerWorld::onKeyUp(std::string_view key, int keyCode) {
  inputController.onKeyUp(key, keyCode);
}

void LayerWorld::onMouseHover(int x, int y) {
  inputController.updateAimFromMouse(x, y);
  UiLayer::onMouseHover(x, y);
}

void LayerWorld::onMouseDown(int x, int y, int button) {
  if (inputController.onMouseDown(x, y, button)) {
    return;
  }
  UiLayer::onMouseDown(x, y, button);
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

void LayerWorld::syncFromState() {
  if (!assertInterfaces()) {
    remove();
    return;
  }
  viewSync.refresh();
  alignMapView();
}

void LayerWorld::update(int deltaTime) {
  UiLayer::update(deltaTime);

  // Hover is polled here: LayerManager has no mouse-move dispatch, and tests/game
  // only wire down/up/wheel. mouseX/Y are updated by SDL every frame.
  auto& events = window->getEvents();
  inputController.updateAimFromMouse(events.mouseX, events.mouseY);
  inputController.updateHeldMoveRepeat(deltaTime);

  auto stateManager = getStateManager();
  if (stateManager) {
    worldUpdate(window, *stateManager, deltaTime);
    state::worldProcessPendingTriggers(window, *stateManager);
    if (stateManager->getState().triggers.mapChangedThisTick) {
      syncFromState();
    }
  }

  viewSync.syncWorldActionModeHighlight();
  viewSync.syncActionModeCancelButton();
  viewSync.syncCombatTitleBar();
}

void LayerWorld::render(int deltaTime) {
  // World is SUSPENDED while inventory/pickup is open (update does not run); still
  // refresh those action button pressed states before drawing.
  viewSync.syncWorldActionModeHighlight();
  UiLayer::render(deltaTime);
}

} // namespace layers
