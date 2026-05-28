#pragma once

#include "layers/LayerManager.h"
#include "layers/ui/LayerGiveContext.h"
#include "layers/ui/LayerInventoryContext.h"
#include "lib/sdl2w/L10n.h"
#include "model/Character.h"
#include "model/Player.h"
#include "state/AbstractAction.h"
#include "state/State.h"

namespace state {

namespace actions {

class UiGiveInventoryItem : public AbstractAction {
  std::string fromCharacterPlayerId;
  std::string toCharacterPlayerId;
  std::string itemId;
  int quantity;

  void act() override {
    auto& localState = *state;
    auto* fromMember =
        model::playerFindPartyMemberById(localState.player, fromCharacterPlayerId);
    auto* toMember =
        model::playerFindPartyMemberById(localState.player, toCharacterPlayerId);
    if (!fromMember || !toMember) {
      LOG(WARN) << "UiGiveInventoryItem::act: party member not found" << LOG_ENDL;
      return;
    }

    auto* database = getDatabase();
    if (!database) {
      LOG(WARN) << "UiGiveInventoryItem::act: database is nullptr" << LOG_ENDL;
      return;
    }

    const auto result = model::characterPlayerGiveInventoryItem(
        *fromMember, *toMember, itemId, quantity, *database);

    auto layerManager = getLayerManager();
    if (!layerManager) {
      return;
    }

    switch (result) {
    case model::GiveItemResult::TOO_HEAVY: {
      UiFloatingNotification notification;
      notification.id = model::createRandomId();
      notification.message = TRANSLATE("Too heavy!");
      notification.type = UiFloatingNotificationType::WARNING;
      model::timerStructStart(notification.timer, kFloatingNotificationDurationMs);
      localState.uiState.floatingNotifications.push_back(std::move(notification));

      auto* giveLayer = layerManager->getLayerById(layers::LayerGiveContext::LAYER_ID);
      if (giveLayer) {
        giveLayer->remove();
        layerManager->moveToFront(layerManager->getLastActiveLayer());
      }
      break;
    }
    case model::GiveItemResult::SUCCESS: {
      auto* giveLayer = layerManager->getLayerById(layers::LayerGiveContext::LAYER_ID);
      if (giveLayer) {
        giveLayer->remove();
      }
      auto* inventoryContextLayer =
          layerManager->getLayerById(layers::LayerInventoryContext::LAYER_ID);
      if (inventoryContextLayer) {
        inventoryContextLayer->remove();
      }
      layerManager->moveToFront(layerManager->getLastActiveLayer());
      break;
    }
    default:
      LOG(WARN) << "UiGiveInventoryItem::act: give failed" << LOG_ENDL;
      break;
    }
  }

public:
  UiGiveInventoryItem(std::string _fromCharacterPlayerId,
                      std::string _toCharacterPlayerId,
                      std::string _itemId,
                      int _quantity)
      : fromCharacterPlayerId(std::move(_fromCharacterPlayerId)),
        toCharacterPlayerId(std::move(_toCharacterPlayerId)),
        itemId(std::move(_itemId)),
        quantity(_quantity) {}
};

} // namespace actions

} // namespace state
