#include "LayerSpecialItemContext.h"
#include "sdl2w/Logger.h"
#include "ui/components/FloatingNotificationSection.h"
#include "ui/popups/PopupPickupItem.h"

namespace layers {

LayerSpecialItemContext::LayerSpecialItemContext(sdl2w::Window* _window,
                                                 bmin::String itemTemplateName,
                                                 bmin::String /*quantityText*/)
    : UiLayer(_window, LAYER_ID) {

  if (!assertInterfaces()) {
    remove();
    return;
  }
  if (itemTemplateName.empty()) {
    LOG(ERROR) << "LayerSpecialItemContext: itemTemplateName is empty" << LOG_ENDL;
    return;
  }

  auto database = getDatabase();
  auto& itemTemplate = database->getItemTemplate(bmin::toStringView(itemTemplateName));

  auto [windowWidth, windowHeight] = window->getDims();
  const auto orientation =
      windowWidth < 500 ? ui::PopupOrientation::NARROW : ui::PopupOrientation::WIDE;

  auto popupPickupItem =
      new ui::PopupPickupItem(window, state::LayerId::SpecialItemContext, orientation);
  popupPickupItem->setId("popupPickupItem");

  ui::PopupPickupItemProps popupProps;
  popupProps.spriteName = itemTemplate.iconSpriteName;
  popupProps.label = itemTemplate.label.empty() ? itemTemplate.name : itemTemplate.label;
  popupProps.description = itemTemplate.description;
  popupProps.showWeightAndValue = false;
  popupProps.orientation = orientation;
  popupPickupItem->setProps(popupProps);

  auto [popupW, popupH] = popupPickupItem->getDims();
  popupPickupItem->setPos((windowWidth - popupW) / 2, (windowHeight - popupH) / 2);
  popupPickupItem->setScale(1.0f);
  popupPickupItem->build();

  addUiElement(bmin::UniquePtr<ui::UiElement>(popupPickupItem));

  auto floatingNotificationSection = new ui::FloatingNotificationSection(window);
  floatingNotificationSection->setId("floatingNotificationSection");
  addUiElement(bmin::UniquePtr<ui::UiElement>(floatingNotificationSection));
}

void LayerSpecialItemContext::update(int deltaTime) { UiLayer::update(deltaTime); }

void LayerSpecialItemContext::render(int deltaTime) { UiLayer::render(deltaTime); }

} // namespace layers
