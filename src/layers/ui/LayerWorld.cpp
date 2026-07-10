#include "LayerWorld.h"
#include "sdl2w/Logger.h"
#include "model/instances/CharacterPlayer.h"
#include "state/WorldActions.h"
#include "ui/components/FloatingNotificationSection.h"
#include "ui/components/InGameTitleBar.h"
#include "ui/layouts/InGameLayout.h"

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

  // TODO: map rendering — draw into the layout content region via
  // InGameLayout::getWorldLocation() / border content area (no map widget yet).

  addUiElement(inGameLayout);

  auto floatingNotificationSection = new ui::FloatingNotificationSection(window);
  floatingNotificationSection->setId("floatingNotificationSection");
  addUiElement(floatingNotificationSection);

  syncFromState();
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
}

} // namespace layers
