#pragma once

#include "state/StateManager.h"
#include "state/actions/ui/UiGiveInventoryItem.hpp"
#include "ui/UiElement.h"
#include "ui/popups/PopupGive.h"
#include "bmin/String.h"

namespace ui {

class ObserverGiveInventoryItem : public ui::UiEventObserver,
                                  public state::StateManagerInterface {
  bmin::String toCharacterPlayerId;
  bmin::String fromCharacterPlayerId;
  bmin::String itemId;
  PopupGive* popupGive;

public:
  ObserverGiveInventoryItem(const bmin::String& _toCharacterPlayerId,
                            const bmin::String& _fromCharacterPlayerId,
                            const bmin::String& _itemId,
                            PopupGive* _popupGive)
      : toCharacterPlayerId(_toCharacterPlayerId),
        fromCharacterPlayerId(_fromCharacterPlayerId),
        itemId(_itemId),
        popupGive(_popupGive) {}

  void onClick(int mouseX, int mouseY, int button) override {
    LOG(INFO) << "ObserverGiveInventoryItem::onClick to=" << toCharacterPlayerId
              << LOG_ENDL;
    auto stateManager = getStateManager();
    if (!stateManager || popupGive == nullptr) {
      return;
    }
    const int quantity = popupGive->getSelectedQuantity();
    stateManager->enqueueAction(
        stateManager->getActionData(),
        new state::actions::UiGiveInventoryItem(
            fromCharacterPlayerId, toCharacterPlayerId, itemId, quantity),
        0);
  }
};

} // namespace ui
