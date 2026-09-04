module;
#include <string_view>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <algorithm>
#include <typeinfo>
#include <typeindex>

export module carcer.layers:LayerGiveContext;
export import carcer.layers.Layer;
import sdl2w;
import carcer.model.instances;
import carcer.ui.components;
import carcer.ui.popups;
import bmin.containers;
import bmin.string_interop;
#include "macros.h"

export {

namespace layers {

class LayerGiveContext : public Layer {
public:
  explicit LayerGiveContext(sdl2w::Window* _window,
                            bmin::String fromCharacterPlayerId,
                            bmin::String itemId);
  ~LayerGiveContext() override = default;

  void update(int deltaTime) override;
  void render(int deltaTime) override;
};

} // namespace layers

} // export

namespace layers {

LayerGiveContext::LayerGiveContext(sdl2w::Window* _window,
                                   bmin::String fromCharacterPlayerId,
                                   bmin::String itemId)
    : Layer(_window, state::LayerId::GiveContext) {

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

  popupGive->setScale(1.f);
  auto [popupW, popupH] = popupGive->getDims();
  popupGive->setPos((windowWidth - popupW) / 2, (windowHeight - popupH) / 2);
  popupGive->build();

  addUiElement(popupGive);

  auto floatingNotificationSection = new ui::FloatingNotificationSection(window);
  floatingNotificationSection->setId("floatingNotificationSection");
  addUiElement(floatingNotificationSection);
}

void LayerGiveContext::update(int deltaTime) { Layer::update(deltaTime); }

void LayerGiveContext::render(int deltaTime) { Layer::render(deltaTime); }

} // namespace layers
