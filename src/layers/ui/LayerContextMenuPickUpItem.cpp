#include "LayerContextMenuPickUpItem.h"
#include "ui/popups/PopupPickupItem.h"

namespace layers {

LayerContextMenuPickUpItem::LayerContextMenuPickUpItem(sdl2w::Window* _window,
                                                       std::string itemId,
                                                       std::string itemName)
    : Layer(_window) {

  if (itemId.empty() || itemName.empty()) {
    LOG(ERROR) << "LayerContextMenuPickUpItem::LayerContextMenuPickUpItem: itemId "
                  "or itemName is empty"
               << LOG_ENDL;
    return;
  }

  auto popupPickupItem = std::make_unique<ui::PopupPickupItem>(window, this);
  popupPickupItem->setId("popupPickupItem");

  auto [windowWidth, windowHeight] = window->getDims();

  ui::BaseStyle style = popupPickupItem->getStyle();
  style.width = std::min(windowWidth, 400);
  style.x = (windowWidth - style.width) / 2;
  style.scale = 1.0f;
  popupPickupItem->setStyle(style);

  ui::PopupPickupItemProps popupProps;
  popupProps.itemName = itemName;
  popupProps.itemId = itemId;
  popupPickupItem->setProps(popupProps);

  style.y = windowHeight / 2 - popupPickupItem->getDims().second / 2;
  popupPickupItem->setStyle(style);

  addUiElement(std::move(popupPickupItem));
}

void LayerContextMenuPickUpItem::update(int deltaTime) { Layer::update(deltaTime); }

void LayerContextMenuPickUpItem::render(int deltaTime) { Layer::render(deltaTime); }

} // namespace layers

