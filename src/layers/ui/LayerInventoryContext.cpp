#include "LayerInventoryContext.h"
#include "sdl2w/Logger.h"
#include "model/instances/CharacterPlayer.h"
#include "ui/components/FloatingNotificationSection.h"
#include "ui/popups/PopupInventoryItem.h"

namespace layers {

LayerInventoryContext::LayerInventoryContext(sdl2w::Window* _window,
                                             bmin::String itemId,
                                             bmin::String itemName)
    : Layer(_window, LAYER_ID) {

  if (!assertInterfaces()) {
    remove();
    return;
  }

  if (itemId.empty() || itemName.empty()) {
    LOG(ERROR) << "LayerInventoryContext::LayerInventoryContext: itemId "
                  "or itemName is empty"
               << LOG_ENDL;
    return;
  }

  auto database = getDatabase();
  auto stateManager = getStateManager();
  auto& player = stateManager->getState().player;
  auto* inventoryPartyMember = model::playerFindPartyMemberByIndex(
      player, player.currentPartyMemberInventoryIndex);

  if (inventoryPartyMember == nullptr) {
    LOG(ERROR) << "LayerInventoryContext::LayerInventoryContext: inventory party "
                  "member is nullptr"
               << LOG_ENDL;
    return;
  }

  model::ItemInstance itemInstance;
  for (const auto& item : inventoryPartyMember->inventory) {
    if (item.id == itemId) {
      itemInstance.id = item.id;
      itemInstance.itemTemplateName = item.itemName;
      itemInstance.quantity = item.quantity;
      break;
    }
  }

  auto& itemTemplate = database->getItemTemplate(bmin::toStringView(itemInstance.itemTemplateName));

  auto [windowWidth, windowHeight] = window->getDims();
  const auto orientation =
      windowWidth < 500 ? ui::PopupOrientation::NARROW : ui::PopupOrientation::WIDE;

  auto popupInventoryItem = new ui::PopupInventoryItem(window, nullptr, orientation);
  popupInventoryItem->setId("popupInventoryItem");

  ui::PopupInventoryItemProps popupProps;
  popupProps.characterPlayerId = inventoryPartyMember->instanceId;
  popupProps.item = {
      .id = itemInstance.id,
      .itemTemplateName = itemInstance.itemTemplateName,
      .quantity = itemInstance.quantity,
  };
  popupProps.spriteName = itemTemplate.iconSpriteName;
  popupProps.label = itemTemplate.label.empty() ? itemTemplate.name : itemTemplate.label;
  popupProps.description = itemTemplate.description;
  popupProps.weight = itemInstance.quantity * itemTemplate.weight;
  popupProps.value = itemInstance.quantity * itemTemplate.value;
  popupProps.orientation = orientation;
  popupInventoryItem->setProps(popupProps);

  auto [popupW, popupH] = popupInventoryItem->getDims();
  popupInventoryItem->setPos((windowWidth - popupW) / 2, (windowHeight - popupH) / 2);
  popupInventoryItem->setScale(1.0f);
  popupInventoryItem->build();

  addUiElement(popupInventoryItem);

  auto floatingNotificationSection = new ui::FloatingNotificationSection(window);
  floatingNotificationSection->setId("floatingNotificationSection");
  addUiElement(floatingNotificationSection);
}

void LayerInventoryContext::update(int deltaTime) { Layer::update(deltaTime); }

void LayerInventoryContext::render(int deltaTime) { Layer::render(deltaTime); }

} // namespace layers
