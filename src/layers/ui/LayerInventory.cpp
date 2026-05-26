#include "LayerInventory.h"
#include "lib/sdl2w/Logger.h"
#include "model/Character.h"
#include "state/actions/ui/UiSetCurrentPartyMember.hpp"
#include "ui/pages/PageInventory.h"

namespace layers {

LayerInventory::LayerInventory(sdl2w::Window* _window) : Layer(_window, LAYER_ID) {
  if (!assertInterfaces()) {
    remove();
    return;
  }

  auto pageInventory = new ui::PageInventory(window);
  pageInventory->setId("pageInventory");

  auto [windowWidth, windowHeight] = window->getDims();
  const auto scale = 1.f;

  auto& style = pageInventory->getStyle();
  style.width = windowWidth / scale;
  style.height = windowHeight / scale;
  style.x = 0;
  style.y = 0;
  style.scale = scale;

  addUiElement(pageInventory);
  syncCurrentPartyMember();

  subscribeAction<state::actions::UiSetCurrentPartyMember>(
      [this](...) { syncCurrentPartyMember(); });
}

void LayerInventory::syncCurrentPartyMember() {
  if (!assertInterfaces()) {
    remove();
    return;
  }

  auto stateManager = getStateManager();
  auto& player = stateManager->getState().player;
  auto currentPartyMember =
      model::playerFindPartyMemberByIndex(player, player.currentPartyMemberIndex);

  if (!currentPartyMember) {
    LOG(ERROR) << "LayerInventory::syncCurrentPartyMember: currentPartyMember is nullptr"
               << LOG_ENDL;
    remove();
    return;
  }

  auto pageInventory = getUiElement<ui::PageInventory>("pageInventory");
  if (!pageInventory) {
    LOG(ERROR) << "LayerInventory::syncCurrentPartyMember: pageInventory is nullptr"
               << LOG_ENDL;
    return;
  }

  auto pageProps = pageInventory->getProps();
  pageProps.characterPlayerId = currentPartyMember->id;
  pageProps.characterPlayerLabel = currentPartyMember->params.label;
  pageProps.partyMemberIndex = player.currentPartyMemberIndex;
  pageProps.characterPlayerSprite =
      characterGetSprite(*currentPartyMember, getDatabase());
  pageProps.weightCarrying =
      model::characterGetWeightCarrying(*currentPartyMember, getDatabase());
  pageProps.weightCapacity = model::characterGetWeightCapacity(*currentPartyMember);
  pageProps.gold = player.gold;
  pageProps.inventory = currentPartyMember->inventory;
  pageInventory->setProps(pageProps);
}

} // namespace layers
