#include "LayerInventoryItemContextMenu.h"
#include "ui/popups/PopupInventoryItem.h"

namespace layers {

LayerInventoryItemContextMenu::LayerInventoryItemContextMenu(sdl2w::Window* _window,
                                                             std::string itemId,
                                                             std::string itemName)
    : Layer(_window) {

  if (itemId.empty() || itemName.empty()) {
    LOG(ERROR) << "LayerInventoryItemContextMenu::LayerInventoryItemContextMenu: itemId "
                  "or itemName is empty"
               << LOG_ENDL;
    return;
  }

  auto popupInventoryItem = std::make_unique<ui::PopupInventoryItem>(window, this);
  popupInventoryItem->setId("popupInventoryItem");

  auto [windowWidth, windowHeight] = window->getDims();

  ui::BaseStyle style = popupInventoryItem->getStyle();
  style.width = std::min(windowWidth, 400);
  style.x = (windowWidth - style.width) / 2;
  style.scale = 1.0f;
  popupInventoryItem->setStyle(style);

  ui::PopupInventoryItemProps popupProps;
  popupProps.itemName = itemName;
  popupProps.itemId = itemId;
  popupInventoryItem->setProps(popupProps);

  style.y = windowHeight / 2 - popupInventoryItem->getDims().second / 2;
  popupInventoryItem->setStyle(style);

  addUiElement(std::move(popupInventoryItem));
}

void LayerInventoryItemContextMenu::update(int deltaTime) { Layer::update(deltaTime); }

void LayerInventoryItemContextMenu::render(int deltaTime) { Layer::render(deltaTime); }

} // namespace layers
