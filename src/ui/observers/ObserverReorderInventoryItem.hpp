#pragma once

#include "state/StateManager.h"
#include "state/actions/ui/UiReorderInventoryItem.hpp"
#include "ui/UiElement.h"

namespace ui {

class ObserverReorderInventoryItem : public ui::UiEventObserver,
                                     public state::StateManagerInterface {
  String characterPlayerId;
  int inventoryIndex;
  int direction;

public:
  ObserverReorderInventoryItem(const String& _characterPlayerId,
                               int _inventoryIndex,
                               int _direction)
      : characterPlayerId(_characterPlayerId),
        inventoryIndex(_inventoryIndex),
        direction(_direction) {}

  void onClick(int mouseX, int mouseY, int button) override {
    LOG(INFO) << "ObserverReorderInventoryItem::onClick index=" << inventoryIndex
              << " direction=" << direction << LOG_ENDL;
    auto stateManager = getStateManager();
    if (!stateManager) {
      return;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(),
        new state::actions::UiReorderInventoryItem(
            characterPlayerId, inventoryIndex, direction),
        0);
  }
};

} // namespace ui
