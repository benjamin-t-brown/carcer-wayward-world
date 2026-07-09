#include "LayerGiveContext.h"
#include "lib/sdl2w/Logger.h"
#include "model/instances/CharacterPlayer.h"
#include "model/instances/Player.h"
#include "ui/components/FloatingNotificationSection.h"
#include "ui/popups/PopupGive.h"

namespace layers {

LayerGiveContext::LayerGiveContext(sdl2w::Window* _window,
                                   String fromCharacterPlayerId,
                                   String itemId)
    : Layer(_window, LAYER_ID) {

  if (!assertInterfaces()) {
    remove();
    return;
  }

  if (fromCharacterPlayerId.empty() || itemId.empty()) {
    LOG(ERROR) << "LayerGiveContext: fromCharacterPlayerId or itemId is empty"
               << LOG_ENDL;
    remove();
    return;
  }

  auto stateManager = getStateManager();
  auto* database = getDatabase();
  if (!database) {
    LOG(ERROR) << "LayerGiveContext: database is nullptr" << LOG_ENDL;
    remove();
    return;
  }
  auto& player = stateManager->getState().player;
  auto* fromMember = model::playerFindPartyMemberById(player, fromCharacterPlayerId);
  if (fromMember == nullptr) {
    LOG(ERROR) << "LayerGiveContext: from party member not found" << LOG_ENDL;
    remove();
    return;
  }

  model::ItemInstance itemInstance;
  for (const auto& item : fromMember->inventory) {
    if (item.id == itemId) {
      itemInstance.id = item.id;
      itemInstance.itemTemplateName = item.itemName;
      itemInstance.quantity = item.quantity;
      break;
    }
  }
  if (itemInstance.id.empty()) {
    LOG(ERROR) << "LayerGiveContext: item not found in inventory" << LOG_ENDL;
    remove();
    return;
  }

  auto [windowWidth, windowHeight] = window->getDims();

  auto popupGive = new ui::PopupGive(window, nullptr);
  popupGive->setId("popupGive");

  ui::PopupGiveProps popupProps;
  popupProps.fromCharacterPlayerId = fromCharacterPlayerId;
  popupProps.itemId = itemId;
  {
    const auto& itemTemplate = database->getItemTemplate(bmin::toStringView(itemInstance.itemTemplateName));
    popupProps.itemLabel =
        itemTemplate.label.empty() ? itemTemplate.name : itemTemplate.label;
  }
  popupProps.maxQuantity = itemInstance.quantity;
  popupProps.selectedQuantity = std::max(1, itemInstance.quantity);
  popupProps.showQuantitySlider =
      database->getItemTemplate(bmin::toStringView(itemInstance.itemTemplateName)).stackable &&
      itemInstance.quantity > 1;
  for (const auto& member : player.party) {
    const auto label = member.params.label.empty() ? member.name : member.params.label;
    popupProps.partyMembers.pushBack(
        {.characterPlayerId = member.instanceId,
         .label = label,
         .spriteName = model::characterPlayerGetSprite(member)});
  }
  popupGive->setProps(popupProps);

  auto& style = popupGive->getStyle();
  style.scale = 1.f;
  auto [popupW, popupH] = popupGive->getDims();
  style.x = (windowWidth - popupW) / 2;
  style.y = (windowHeight - popupH) / 2;
  popupGive->build();

  addUiElement(popupGive);

  auto floatingNotificationSection = new ui::FloatingNotificationSection(window);
  floatingNotificationSection->setId("floatingNotificationSection");
  addUiElement(floatingNotificationSection);
}

void LayerGiveContext::update(int deltaTime) { Layer::update(deltaTime); }

void LayerGiveContext::render(int deltaTime) { Layer::render(deltaTime); }

} // namespace layers
