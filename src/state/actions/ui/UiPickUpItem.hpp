#pragma once

#include "bmin/String.h"
#include "bmin/StringInterop.h"
#include "model/instances/CharacterPlayer.h"
#include "model/instances/Player.h"
#include "model/templates/Items.h"
#include "model/templates/UtilityTypes.h"
#include "sdl2w/L10n.h"
#include "state/AbstractAction.h"
#include "state/State.h"

namespace state {

namespace actions {

class UiPickUpItem : public AbstractAction {
  bmin::String itemId;

  void act() override {
    if (!state) {
      return;
    }
    auto& localState = *state;
    auto* database = getDatabase();
    if (!database) {
      LOG(WARN) << "UiPickUpItem::act: database is nullptr" << LOG_ENDL;
      return;
    }

    auto* partyMember = model::playerFindPartyMemberByIndex(
        localState.player, localState.player.currentPartyMemberIndex);
    if (!partyMember) {
      LOG(WARN) << "UiPickUpItem::act: party member not found" << LOG_ENDL;
      return;
    }

    model::ItemInstance* mapItem = nullptr;
    size_t mapItemIndex = 0;
    for (size_t i = 0; i < localState.world.activeMap.items.size(); i++) {
      if (localState.world.activeMap.items[i].id == itemId) {
        mapItem = &localState.world.activeMap.items[i];
        mapItemIndex = i;
        break;
      }
    }
    if (!mapItem) {
      LOG(WARN) << "UiPickUpItem::act: item not found on map" << LOG_ENDL;
      return;
    }

    const model::ItemTemplate* itemTemplate = nullptr;
    try {
      itemTemplate =
          &database->getItemTemplate(bmin::toStringView(mapItem->itemTemplateName));
    } catch (...) {
      LOG(WARN) << "UiPickUpItem::act: item template not found "
                << mapItem->itemTemplateName << LOG_ENDL;
      return;
    }

    const int addedWeight = mapItem->quantity * itemTemplate->weight;
    if (model::characterGetWeightCarrying(*partyMember, database) + addedWeight >
        model::characterGetWeightCapacity(*partyMember)) {
      UiFloatingNotification notification;
      notification.id = model::createRandomId();
      notification.message = TRANSLATE("Too heavy!");
      notification.type = UiFloatingNotificationType::WARNING;
      model::timerStructStart(notification.timer,
                              localState.settings.floatingNotificationDurationMs);
      localState.uiState.floatingNotifications.pushBack(std::move(notification));
      return;
    }

    model::characterPlayerAddItemToInventory(
        *partyMember, *itemTemplate, mapItem->quantity);
    localState.world.activeMap.items.erase(mapItemIndex);
  }

public:
  explicit UiPickUpItem(bmin::String _itemId) : itemId(std::move(_itemId)) {}
};

} // namespace actions

} // namespace state
