#include "LayerInventory.h"
#include "sdl2w/Logger.h"
#include "model/instances/CharacterPlayer.h"
#include "state/actions/ui/UiReorderInventoryItem.hpp"
#include "state/actions/ui/UiSetCurrentPartyMemberInventory.hpp"
#include "state/actions/ui/UiDropInventoryItem.hpp"
#include "state/actions/ui/UiGiveInventoryItem.hpp"
#include "state/actions/ui/UiToggleEquipInventoryItem.hpp"
#include "ui/components/FloatingNotificationSection.h"
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

  pageInventory->setPos(0, 0);
  pageInventory->setScale(scale);
  auto pageInitProps = pageInventory->getProps();
  pageInitProps.width = static_cast<int>(windowWidth / scale);
  pageInitProps.height = static_cast<int>(windowHeight / scale);
  pageInventory->setProps(pageInitProps);

  addUiElement(pageInventory);

  auto floatingNotificationSection = new ui::FloatingNotificationSection(window);
  floatingNotificationSection->setId("floatingNotificationSection");
  addUiElement(floatingNotificationSection);

  syncInventoryPartyMember();

  subscribeAction<state::actions::UiSetCurrentPartyMemberInventory>(
      [this](auto&, auto&) { syncInventoryPartyMember(); });
  subscribeAction<state::actions::UiReorderInventoryItem>(
      [this](auto&, auto&) { syncInventoryPartyMember(); });
  subscribeAction<state::actions::UiToggleEquipInventoryItem>(
      [this](auto&, auto&) { syncInventoryPartyMember(); });
  subscribeAction<state::actions::UiGiveInventoryItem>(
      [this](auto&, auto&) { syncInventoryPartyMember(); });
  subscribeAction<state::actions::UiDropInventoryItem>(
      [this](auto&, auto&) { syncInventoryPartyMember(); });
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
  pageProps.characterPlayerId = inventoryPartyMember->instanceId;
  pageProps.characterPlayerLabel = inventoryPartyMember->params.label;
  pageProps.partyMemberInventoryIndex = player.currentPartyMemberInventoryIndex;
  pageProps.partyMembers.clear();
  for (const auto& member : player.party) {
    pageProps.partyMembers.pushBack(
        {.spriteName = model::characterPlayerGetSprite(member)});
  }
  pageProps.characterPlayerSprite = model::characterPlayerGetSprite(*inventoryPartyMember);
  pageProps.weightCarrying =
      model::characterGetWeightCarrying(*inventoryPartyMember, getDatabase());
  pageProps.weightCapacity = model::characterGetWeightCapacity(*inventoryPartyMember);
  pageProps.gold = player.gold;
  pageProps.inventory = inventoryPartyMember->inventory;
  pageProps.equipment = inventoryPartyMember->equipment;
  pageInventory->setProps(pageProps);
}

} // namespace layers
