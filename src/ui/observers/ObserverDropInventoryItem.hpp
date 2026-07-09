#pragma once

#include "state/StateManager.h"
#include "state/actions/ui/UiDropInventoryItem.hpp"
#include "ui/UiElement.h"

namespace ui {

class ObserverDropInventoryItem : public ui::UiEventObserver,
                                  public state::StateManagerInterface {
  String characterPlayerId;
  String itemId;

public:
  ObserverDropInventoryItem(const String& _characterPlayerId, const String& _itemId)
      : characterPlayerId(_characterPlayerId), itemId(_itemId) {}

  void onClick(int mouseX, int mouseY, int button) override {
    LOG(INFO) << "ObserverDropInventoryItem::onClick item=" << itemId << LOG_ENDL;
    auto stateManager = getStateManager();
    if (!stateManager) {
      return;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(),
        new state::actions::UiDropInventoryItem(characterPlayerId, itemId),
        0);
  }
};

} // namespace ui
