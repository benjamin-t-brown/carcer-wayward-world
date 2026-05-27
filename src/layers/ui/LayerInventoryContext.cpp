#include "LayerInventoryContext.h"
#include "lib/sdl2w/Logger.h"
#include "model/Character.h"
#include "ui/popups/PopupInventoryItem.h"

namespace layers {

LayerInventoryContext::LayerInventoryContext(sdl2w::Window* _window,
                                             std::string itemId,
                                             std::string itemName)
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
      itemInstance.itemName = item.itemName;
      itemInstance.quantity = item.quantity;
      break;
    }
  }

  auto& itemTemplate = database->getItemTemplate(itemInstance.itemName);

  auto [windowWidth, windowHeight] = window->getDims();
  const auto orientation =
      windowWidth < 500 ? ui::PopupOrientation::NARROW : ui::PopupOrientation::WIDE;

  auto popupInventoryItem = new ui::PopupInventoryItem(window, nullptr, orientation);
  popupInventoryItem->setId("popupInventoryItem");

  auto& style = popupInventoryItem->getStyle();
  style.x = (windowWidth - style.width) / 2;
  style.y = (windowHeight - style.height) / 2;
  style.scale = 1.0f;

  ui::PopupInventoryItemProps popupProps;
  popupProps.item = {
      .id = itemInstance.id,
      .itemName = itemInstance.itemName,
      .quantity = itemInstance.quantity,
  };
  popupProps.spriteName = itemTemplate.iconSpriteName;
  popupProps.label = itemTemplate.label.empty() ? itemTemplate.name : itemTemplate.label;
  popupProps.description = itemTemplate.description;
  popupProps.weight = itemInstance.quantity * itemTemplate.weight;
  popupProps.value = itemInstance.quantity * itemTemplate.value;
  popupProps.orientation = orientation;
  popupInventoryItem->setProps(popupProps);

  addUiElement(popupInventoryItem);
}

void LayerInventoryContext::update(int deltaTime) { Layer::update(deltaTime); }

void LayerInventoryContext::render(int deltaTime) { Layer::render(deltaTime); }

} // namespace layers
