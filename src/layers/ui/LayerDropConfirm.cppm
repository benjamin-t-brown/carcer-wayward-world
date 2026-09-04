module;
#include <string_view>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <typeinfo>
#include <typeindex>

export module carcer.layers.LayerDropConfirm;
export import carcer.layers.Layer;
import sdl2w;
import carcer.model.instances.ItemInstance;
import carcer.model.instances.Player;
import carcer.ui.components;
import carcer.ui.popups;
import bmin.containers;
import bmin.string_interop;
#include "macros.h"

export {

namespace layers {

class LayerDropConfirm : public Layer {
public:
  explicit LayerDropConfirm(sdl2w::Window* _window,
                            bmin::String characterPlayerId,
                            bmin::String itemId);
  ~LayerDropConfirm() override = default;

  void update(int deltaTime) override;
  void render(int deltaTime) override;
};

} // namespace layers

} // export

namespace layers {

LayerDropConfirm::LayerDropConfirm(sdl2w::Window* _window,
                                   bmin::String characterPlayerId,
                                   bmin::String itemId)
    : Layer(_window, state::LayerId::DropConfirm) {

  if (!assertInterfaces()) {
    remove();
    return;
  }

  if (characterPlayerId.empty() || itemId.empty()) {
    LOG(ERROR) << "LayerDropConfirm: characterPlayerId or itemId is empty" << LOG_ENDL;
    remove();
    return;
  }

  auto stateManager = getStateManager();
  auto* database = getDatabase();
  if (!database) {
    LOG(ERROR) << "LayerDropConfirm: database is nullptr" << LOG_ENDL;
    remove();
    return;
  }
  auto& player = stateManager->getState().player;
  auto* partyMember = model::playerFindPartyMemberById(player, characterPlayerId);
  if (partyMember == nullptr) {
    LOG(ERROR) << "LayerDropConfirm: party member not found" << LOG_ENDL;
    remove();
    return;
  }

  model::ItemInstance itemInstance;
  for (const auto& item : partyMember->inventory) {
    if (item.id == itemId) {
      itemInstance.id = item.id;
      itemInstance.itemTemplateName = item.itemName;
      itemInstance.quantity = item.quantity;
      break;
    }
  }
  if (itemInstance.id.empty()) {
    LOG(ERROR) << "LayerDropConfirm: item not found in inventory" << LOG_ENDL;
    remove();
    return;
  }

  auto [windowWidth, windowHeight] = window->getDims();

  auto popup = new ui::PopupDropConfirm(window, nullptr);
  popup->setId("popupDropConfirm");

  ui::PopupDropConfirmProps popupProps;
  popupProps.characterPlayerId = characterPlayerId;
  popupProps.itemId = itemId;
  {
    const auto& itemTemplate = database->getItemTemplate(bmin::toStringView(itemInstance.itemTemplateName));
    popupProps.itemLabel =
        itemTemplate.label.empty() ? itemTemplate.name : itemTemplate.label;
  }
  popup->setProps(popupProps);

  popup->setScale(1.f);
  auto [popupW, popupH] = popup->getDims();
  popup->setPos((windowWidth - popupW) / 2, (windowHeight - popupH) / 2);
  popup->build();

  addUiElement(popup);

  auto floatingNotificationSection = new ui::FloatingNotificationSection(window);
  floatingNotificationSection->setId("floatingNotificationSection");
  addUiElement(floatingNotificationSection);
}

void LayerDropConfirm::update(int deltaTime) { Layer::update(deltaTime); }

void LayerDropConfirm::render(int deltaTime) { Layer::render(deltaTime); }

} // namespace layers
