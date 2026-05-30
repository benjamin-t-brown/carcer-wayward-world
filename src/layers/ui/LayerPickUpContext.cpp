#include "LayerPickUpContext.h"
#include "ui/components/FloatingNotificationSection.h"
#include "ui/popups/PopupPickupItem.h"

namespace layers {

LayerPickUpContext::LayerPickUpContext(sdl2w::Window* _window,
                                       const model::ItemInstance& item)
    : Layer(_window), item(item) {

  if (!assertInterfaces()) {
    remove();
    return;
  }
  if (item.itemTemplateName.empty() || item.id.empty()) {
    LOG(ERROR) << "LayerPickUpContext::LayerPickUpContext: itemId "
                  "or itemName is empty"
               << LOG_ENDL;
    return;
  }
  auto database = getDatabase();
  auto& itemTemplate = database->getItemTemplate(item.itemTemplateName);

  auto [windowWidth, windowHeight] = window->getDims();
  const auto orientation =
      windowWidth < 500 ? ui::PopupOrientation::NARROW : ui::PopupOrientation::WIDE;

  auto popupPickupItem = new ui::PopupPickupItem(window, this, orientation);
  popupPickupItem->setId("popupPickupItem");

  auto& style = popupPickupItem->getStyle();
  style.x = (windowWidth - style.width) / 2;
  style.y = (windowHeight - style.height) / 2;
  style.scale = 1.0f;

  ui::PopupPickupItemProps popupProps;
  popupProps.spriteName = itemTemplate.iconSpriteName;
  popupProps.label = itemTemplate.label.empty() ? itemTemplate.name : itemTemplate.label;
  popupProps.description = itemTemplate.description;
  popupProps.weight = item.quantity * itemTemplate.weight;
  popupProps.value = item.quantity * itemTemplate.value;
  popupProps.orientation = orientation;
  popupPickupItem->setProps(popupProps);

  addUiElement(popupPickupItem);

  auto floatingNotificationSection = new ui::FloatingNotificationSection(window);
  floatingNotificationSection->setId("floatingNotificationSection");
  addUiElement(floatingNotificationSection);
}

void LayerPickUpContext::update(int deltaTime) { Layer::update(deltaTime); }

void LayerPickUpContext::render(int deltaTime) { Layer::render(deltaTime); }

} // namespace layers
