#include "LayerContextMenuInventoryItem.h"
#include "ui/popups/PopupInventoryItem.h"

namespace layers {

LayerContextMenuInventoryItem::LayerContextMenuInventoryItem(sdl2w::Window* _window,
                                                             std::string itemId,
                                                             std::string itemName)
    : Layer(_window) {

  if (itemId.empty() || itemName.empty()) {
    LOG(ERROR) << "LayerContextMenuInventoryItem::LayerContextMenuInventoryItem: itemId "
                  "or itemName is empty"
               << LOG_ENDL;
    return;
  }

  auto popupInventoryItem = std::make_unique<ui::PopupInventoryItem>(window, this);
  popupInventoryItem->setId("popupInventoryItem");

  auto [windowWidth, windowHeight] = window->getDims();

  auto& style = popupInventoryItem->getStyle();
  style.width = std::min(windowWidth, 400);
  style.x = (windowWidth - style.width) / 2;
  style.scale = 1.0f;

  ui::PopupInventoryItemProps popupProps;
  popupProps.spriteName = itemId;
  popupProps.label = itemName;
  popupProps.description = "";
  popupProps.weight = 0;
  popupProps.value = 0;
  popupInventoryItem->setProps(popupProps);

  style.y = windowHeight / 2 - popupInventoryItem->getDims().second / 2;
  popupInventoryItem->setStyle(style);

  addUiElement(popupInventoryItem.release());
}

void LayerContextMenuInventoryItem::update(int deltaTime) { Layer::update(deltaTime); }

void LayerContextMenuInventoryItem::render(int deltaTime) { Layer::render(deltaTime); }

} // namespace layers

