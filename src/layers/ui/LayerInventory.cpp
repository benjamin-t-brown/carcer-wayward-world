#include "LayerInventory.h"
#include "lib/sdl2w/Logger.h"
#include "model/Character.h"
#include "state/actions/ui/UiReorderInventoryItem.hpp"
#include "state/actions/ui/UiSetCurrentPartyMemberInventory.hpp"
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
  syncInventoryPartyMember();

  subscribeAction<state::actions::UiSetCurrentPartyMemberInventory>(
      [this](...) { syncInventoryPartyMember(); });
  subscribeAction<state::actions::UiReorderInventoryItem>(
      [this](...) { syncInventoryPartyMember(); });
}

void LayerInventory::syncInventoryPartyMember() {
  if (!assertInterfaces()) {
    remove();
    return;
  }

  auto stateManager = getStateManager();
  auto& player = stateManager->getState().player;
  auto inventoryPartyMember = model::playerFindPartyMemberByIndex(
      player, player.currentPartyMemberInventoryIndex);

  if (!inventoryPartyMember) {
    LOG(ERROR) << "LayerInventory::syncInventoryPartyMember: party member is nullptr"
               << LOG_ENDL;
    remove();
    return;
  }

  auto pageInventory = getUiElement<ui::PageInventory>("pageInventory");
  if (!pageInventory) {
    LOG(ERROR) << "LayerInventory::syncInventoryPartyMember: pageInventory is nullptr"
               << LOG_ENDL;
    return;
  }

  auto pageProps = pageInventory->getProps();
  pageProps.characterPlayerId = inventoryPartyMember->id;
  pageProps.characterPlayerLabel = inventoryPartyMember->params.label;
  pageProps.partyMemberInventoryIndex = player.currentPartyMemberInventoryIndex;
  pageProps.partyMembers.clear();
  for (const auto& member : player.party) {
    pageProps.partyMembers.push_back(
        {.spriteName = model::characterPlayerGetSprite(member)});
  }
  pageProps.characterPlayerSprite = model::characterPlayerGetSprite(*inventoryPartyMember);
  pageProps.weightCarrying =
      model::characterGetWeightCarrying(*inventoryPartyMember, getDatabase());
  pageProps.weightCapacity = model::characterGetWeightCapacity(*inventoryPartyMember);
  pageProps.gold = player.gold;
  pageProps.inventory = inventoryPartyMember->inventory;
  pageInventory->setProps(pageProps);
}

} // namespace layers
