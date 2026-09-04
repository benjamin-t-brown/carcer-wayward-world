module;
#include <string_view>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <typeinfo>
#include <typeindex>

export module carcer.layers.LayerPickUpContext;
export import carcer.layers.Layer;
export import carcer.model.instances;
import sdl2w;
import carcer.ui.popups;
import carcer.ui.components;
import bmin.string_interop;
#include "macros.h"

export {

namespace layers {

class LayerPickUpContext : public Layer {
private:
  model::ItemInstance item;

public:
  explicit LayerPickUpContext(sdl2w::Window* _window, const model::ItemInstance& item);
  virtual ~LayerPickUpContext() = default;

  void update(int deltaTime) override;
  void render(int deltaTime) override;
};

} // namespace layers

} // export

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
  auto& itemTemplate = database->getItemTemplate(bmin::toStringView(item.itemTemplateName));

  auto [windowWidth, windowHeight] = window->getDims();
  const auto orientation =
      windowWidth < 500 ? ui::PopupOrientation::NARROW : ui::PopupOrientation::WIDE;

  auto popupPickupItem = new ui::PopupPickupItem(window, getId(), orientation);
  popupPickupItem->setId("popupPickupItem");

  ui::PopupPickupItemProps popupProps;
  popupProps.spriteName = itemTemplate.iconSpriteName;
  popupProps.label = itemTemplate.label.empty() ? itemTemplate.name : itemTemplate.label;
  popupProps.description = itemTemplate.description;
  popupProps.weight = item.quantity * itemTemplate.weight;
  popupProps.value = item.quantity * itemTemplate.value;
  popupProps.orientation = orientation;
  popupPickupItem->setProps(popupProps);

  auto [popupW, popupH] = popupPickupItem->getDims();
  popupPickupItem->setPos((windowWidth - popupW) / 2, (windowHeight - popupH) / 2);
  popupPickupItem->setScale(1.0f);
  popupPickupItem->build();

  addUiElement(popupPickupItem);

  auto floatingNotificationSection = new ui::FloatingNotificationSection(window);
  floatingNotificationSection->setId("floatingNotificationSection");
  addUiElement(floatingNotificationSection);
}

void LayerPickUpContext::update(int deltaTime) { Layer::update(deltaTime); }

void LayerPickUpContext::render(int deltaTime) { Layer::render(deltaTime); }

} // namespace layers
